#include "gifc_mainwindow.h"
#include "./ui_gifc_mainwindow.h"
#include "QFFmpegFunctionLib.h"
#include "QSimpleTimeline.h"
#include "QVideoPlayer.h"
#include "QVideoPlayerWidget.h"
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>

GifC_MainWindow::GifC_MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GifC_MainWindow)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    setupSettings_Ui();

    m_videoPlayer = new QVideoPlayer;
    ui->PlayerHolder->addWidget(m_videoPlayer->getMediaWidget());

    m_cropTimeline = new QSimpleTimeline;
    ui->Player_TimelineHolder->addWidget(m_cropTimeline);

    connect(m_videoPlayer, &QVideoPlayer::onPlayerReadyToPlay, this, &GifC_MainWindow::onPlayerReadyToPlay_Ui);
    connect(m_videoPlayer, &QVideoPlayer::onCurrentPositionChanged, this, &GifC_MainWindow::onCurrentPositionChanged_Ui);
    connect(m_videoPlayer, &QVideoPlayer::onPlayEnded, this, &GifC_MainWindow::onPlayEnded_Ui);
    connect(m_cropTimeline, &QSimpleTimeline::cursorValueChanged, this, &GifC_MainWindow::onCursorValueChanged_Ui);
}

GifC_MainWindow::~GifC_MainWindow()
{
    delete ui;
}

void GifC_MainWindow::setupSettings_Ui()
{
    QComboBox *resComboBox = findChild<QComboBox *>("convert_res_box");
    resComboBox->clear();
    resComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoResolutionAsFloat(VideoResolution::R_320)));
    resComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoResolutionAsFloat(VideoResolution::R_640)));
    resComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoResolutionAsFloat(VideoResolution::R_1280)));
    resComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoResolutionAsFloat(VideoResolution::R_1920)));
    resComboBox->setCurrentIndex(1);

    QComboBox *fpsComboBox = findChild<QComboBox *>("convert_fps_box");
    fpsComboBox->clear();
    fpsComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoFrameRateAsFloat(VideoFrameRate::F_5)));
    fpsComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoFrameRateAsFloat(VideoFrameRate::F_10)));
    fpsComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoFrameRateAsFloat(VideoFrameRate::F_15)));
    fpsComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoFrameRateAsFloat(VideoFrameRate::F_20)));
    fpsComboBox->addItem(QString::number(QFFmpegFunctionLib::VideoFrameRateAsFloat(VideoFrameRate::F_30)));
    fpsComboBox->setCurrentIndex(2);
}

void GifC_MainWindow::launchConversion()
{
    statusBar()->showMessage(tr("Converting, please wait ..."), 0);
    statusBar()->repaint();

    bool bIsSuccess = false;
    if (m_videoPlayer) {
        m_videoPlayer->pause();

        const QMediaInfo *mediaInfo = m_videoPlayer->getMediaInfo();
        if (mediaInfo && mediaInfo->filename != "") {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QString timestampString = currentDateTime.toString("yyyyMMdd_hhmmss");

            QString inFile = mediaInfo->filename;
            QString outFile = inFile.split('.')[0] + "_" + timestampString + ".gif";

            // Dividing by 1000 to go from milliseconds to seconds
            float startTrim = float(m_cropTimeline->getRangeLow()) / 1000.F;
            float endTrim = float(m_cropTimeline->getRangeHigh()) / 1000.F;

            // Get the crop area
            const QSizeF cropSizePercent = m_videoPlayer->getMediaWidget()->getCropSizePercent();
            const QSize cropSize = QSize(cropSizePercent.width() * mediaInfo->resolution.width(),
                                         cropSizePercent.height() * mediaInfo->resolution.height());
            const QPointF cropPosPercent = m_videoPlayer->getMediaWidget()->getCropPosPercent();
            const QPoint cropPos = QPoint(cropPosPercent.x() * mediaInfo->resolution.width(),
                                          cropPosPercent.y() * mediaInfo->resolution.height());

            QComboBox *resComboBox = findChild<QComboBox *>("convert_res_box");
            QComboBox *fpsComboBox = findChild<QComboBox *>("convert_fps_box");

            QFFmpegFunctionLib FFmpegLib;
            bIsSuccess = FFmpegLib.trimVideoToGif(inFile,
                                                  outFile,
                                                  startTrim,
                                                  endTrim,
                                                  cropSize,
                                                  cropPos,
                                                  resComboBox->currentText().toInt(),
                                                  fpsComboBox->currentText().toFloat());
        }
    } else {
        qDebug() << "SetupPlayer: Invalid VideoPlayer.";
    }

    if (bIsSuccess) {
        statusBar()->showMessage(tr("Conversion done."), 2000);
    } else {
        statusBar()->showMessage(tr("Convertion failed."), 2000);
    }
}

void GifC_MainWindow::setMediaFile(QString const &Path)
{
    if (m_videoPlayer) {
        if (m_videoPlayer->isMediaFileSupported(Path)) {
            m_videoPlayer->setMediaFile(Path);
            setPlayButtonState(false);

            statusBar()->showMessage(tr("File loaded."), 2000);
            qDebug() << "SetMediaFile: File loaded.";
        } else {
            statusBar()->showMessage(tr("File not supported."), 2000);
            qDebug() << "SetMediaFile: File not supported.";
        }
    } else {
        qDebug() << "SetMediaPath: Invalid VideoPlayer.";
    }
}

void GifC_MainWindow::onCursorValueChanged_Ui()
{
    if (m_videoPlayer) {
        const unsigned int NewPostion = m_cropTimeline->getCursorValue();
        m_videoPlayer->setCurrentPosition(NewPostion);
    } else {
        qDebug() << "on_Player_Timeline_sliderReleased: Invalid VideoPlayer.";
    }
}

void GifC_MainWindow::onPlayerReadyToPlay_Ui()
{
    if (m_videoPlayer) {
        qint64 const Duration = m_videoPlayer->getMediaInfo()->duration;

        m_cropTimeline->setCursorValue(0);
        m_cropTimeline->setRangeMinMax(0, Duration);
        m_cropTimeline->setRangeLow(0);
        m_cropTimeline->setRangeHigh(Duration);
    } else {
        qDebug() << "SetupPlayer: Invalid VideoPlayer.";
    }
}

void GifC_MainWindow::onPlayEnded_Ui()
{
    setPlayButtonState(false);
}

void GifC_MainWindow::onCurrentPositionChanged_Ui(unsigned int const Position)
{
    m_cropTimeline->setCursorValue(Position);
}

void GifC_MainWindow::setPlayButtonState(bool const bIsPlaying)
{
    QIcon ButtonIcon(bIsPlaying ? ICON_PAUSE : ICON_PLAY);
    ui->Player_Play->setIcon(ButtonIcon);
}

void GifC_MainWindow::setLoopButtonState(bool const bIsLooping)
{
    QIcon ButtonIcon(bIsLooping ? ICON_NOLOOP : ICON_LOOP);
    ui->Player_Loop->setIcon(ButtonIcon);
}

void GifC_MainWindow::dragEnterEvent(QDragEnterEvent* event)
	{
	// if some actions should not be usable, like move, this code must be adopted
	event->acceptProposedAction();
	}

void GifC_MainWindow::dragMoveEvent(QDragMoveEvent* event)
	{
	// if some actions should not be usable, like move, this code must be adopted
	event->acceptProposedAction();
	}

void GifC_MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}

void GifC_MainWindow::dropEvent(QDropEvent *event)
{
    qDebug() << "dropEvent: File dropped.";

    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        if (!urlList.isEmpty()) {
            QString const fileName = urlList.at(0).toLocalFile();

            setMediaFile(fileName);
            ui->Path_TextField->setText(fileName);
            ui->Path_TextField->clearFocus();
            ui->Path_TextField->deselect();

            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void GifC_MainWindow::on_Path_BrowseButton_released()
{
    const QString fileName = QFileDialog::getOpenFileName(this,
                                                          tr("Choose Video"),
                                                          "",
                                                          tr("Video files (*)"));
    ui->Path_TextField->setText(fileName);
    ui->Path_TextField->clearFocus();
    ui->Path_TextField->deselect();

    setMediaFile(fileName);
}

void GifC_MainWindow::on_Path_TextField_returnPressed()
{
    const QString fileName = ui->Path_TextField->text();
    ui->Path_TextField->clearFocus();
    ui->Path_TextField->deselect();

    setMediaFile(fileName);
}

void GifC_MainWindow::on_Player_Play_released()
{
    if (m_videoPlayer) {
        if (m_videoPlayer->getIsPlaying()) {
            m_videoPlayer->pause();
            setPlayButtonState(false);
        } else {
            m_videoPlayer->play();
            setPlayButtonState(true);
        }
    } else {
        qDebug() << "on_Player_Play_released: Invalid VideoPlayer.";
    }
}

void GifC_MainWindow::on_Player_CropStart_released()
{
    m_cropTimeline->setRangeLowFromCursorValue();
}

void GifC_MainWindow::on_Player_CropEnd_released()
{
    m_cropTimeline->setRangeHighFromCursorValue();
}

void GifC_MainWindow::on_convertBtn_released()
{
    launchConversion();
}

void GifC_MainWindow::on_Player_Loop_released()
{
    if (m_videoPlayer) {
        const bool bIsLooping = !m_videoPlayer->getIsLooping();

        if (bIsLooping) {
            m_videoPlayer->setLoopStartEnd(m_cropTimeline->getRangeLow(), m_cropTimeline->getRangeHigh());
            connect(m_cropTimeline, &QSimpleTimeline::rangeChanged, m_videoPlayer, &QVideoPlayer::setLoopStartEnd);
        } else {
            disconnect(m_cropTimeline, &QSimpleTimeline::rangeChanged, m_videoPlayer, &QVideoPlayer::setLoopStartEnd);
        }

        m_videoPlayer->setIsLooping(bIsLooping);
        setLoopButtonState(bIsLooping);
        statusBar()->showMessage(tr(bIsLooping ? "Loop in range enabled." : "Loop in range disabled."), 2000);

    } else {
        qDebug() << "on_Player_Loop_released: Invalid VideoPlayer.";
    }
}

void GifC_MainWindow::on_Player_GoToStart_released()
{
    if (m_videoPlayer) {
        m_videoPlayer->setCurrentPosition(m_videoPlayer->getCurrentPosition() - 1000);

    } else {
        qDebug() << "on_Player_GoToStart_released: Invalid VideoPlayer.";
    }
}

void GifC_MainWindow::on_Player_GoToEnd_released()
{
    if (m_videoPlayer) {
        m_videoPlayer->setCurrentPosition(m_videoPlayer->getCurrentPosition() + 1000);

    } else {
        qDebug() << "on_Player_GoToStart_released: Invalid VideoPlayer.";
    }
}
