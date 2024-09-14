#ifndef GIFC_MAINWINDOW_H
#define GIFC_MAINWINDOW_H

#include <QMainWindow>

class QVideoPlayer;
class QSimpleTimeline;

QT_BEGIN_NAMESPACE
namespace Ui { class GifC_MainWindow; }
QT_END_NAMESPACE

class GifC_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    GifC_MainWindow(QWidget *parent = nullptr);
    ~GifC_MainWindow();

    void setMediaFile(QString const &Path);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dropEvent(QDropEvent *event);

private slots:
    void on_Path_BrowseButton_released();

    void on_Path_TextField_returnPressed();

    void on_Player_Play_released();

    void on_Player_CropStart_released();

    void on_Player_CropEnd_released();

    void on_Player_Loop_released();

    void on_Player_GoToStart_released();

    void on_Player_GoToEnd_released();

    void on_convertBtn_released();

    void on_exportVidBtn_released();

private:
    void setupSettings_Ui();
    void launchConversion(bool const bSaveAsVideo);

    void onCurrentPositionChanged_Ui(unsigned int const Position);
    void onCursorValueChanged_Ui();
    void onPlayerReadyToPlay_Ui();
    void onPlayEnded_Ui();

    void setPlayButtonState(bool const bIsPlaying);
    void setLoopButtonState(bool const bIsLooping);

    Ui::GifC_MainWindow *ui;
    QVideoPlayer *m_videoPlayer = nullptr;
    QSimpleTimeline *m_cropTimeline;

    QString ICON_PAUSE = ":/icons/Assets/Icons/icons8_pause_64px.png";
    QString ICON_PLAY = ":/icons/Assets/Icons/icons8_play_64px.png";
    QString ICON_LOOP = ":/icons/Assets/Icons/icons8_repeat_64px.png";
    QString ICON_NOLOOP = ":/icons/Assets/Icons/icons8_norepeat_64px.png";
};
#endif // GIFC_MAINWINDOW_H
