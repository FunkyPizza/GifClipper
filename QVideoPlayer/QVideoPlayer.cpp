#include "QVideoPlayer.h"
#include "QVideoPlayerWidget.h"
#include <QDebug>
#include <QGraphicsVideoItem>
#include <QMediaFormat>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMimeDatabase>

QVideoPlayer::QVideoPlayer(QObject *parent)
    : QObject{parent}
{
    initialiseMediaPlayer();
}

QVideoPlayer::~QVideoPlayer()
{
    if (m_mediaPlayerPtr) {
        m_mediaPlayerPtr->stop();
        m_mediaPlayerPtr->setVideoOutput(nullptr);
        delete m_mediaPlayerPtr;
    }
    if (m_mediaWidgetPtr) {
        delete m_mediaWidgetPtr;
    }
}

void QVideoPlayer::initialiseMediaPlayer()
{
    m_mediaWidgetPtr = new QVideoPlayerWidget;
    m_mediaPlayerPtr = new QMediaPlayer;
    m_mediaPlayerPtr->setVideoOutput(m_mediaWidgetPtr->getVideoOutput());

    connect(m_mediaPlayerPtr, &QMediaPlayer::positionChanged, this, &QVideoPlayer::onCurrentPositionChanged_Internal);
}

void QVideoPlayer::onCurrentPositionChanged_Internal(unsigned int const CurrentPosition)
{
    emit onCurrentPositionChanged(CurrentPosition);
    if (m_bIsLooping) {
        if (CurrentPosition >= m_loopEnd) {
            setCurrentPosition(m_loopStart);
        }
    }
    if (CurrentPosition == getMediaInfo()->duration) {
        emit onPlayEnded();
    }
}

void QVideoPlayer::onMediaStatusChanged_Internal(QMediaPlayer::MediaStatus status)
{
    //qDebug() << "OnMediaStatusChanged_Internal: Status changed";
    //qDebug() << status;

    if (status == QMediaPlayer::MediaStatus::LoadedMedia) {
        // Auto play to frame 0 to display first frame
        m_mediaPlayerPtr->play();
        m_mediaPlayerPtr->pause();
        m_mediaPlayerPtr->setPosition(0);

        emit onPlayerReadyToPlay();
        // Initialise crop feature
        m_mediaWidgetPtr->initialiseFromNewMedia();
    }
}

QVideoPlayerWidget *const QVideoPlayer::getMediaWidget()
{
    if (!m_mediaWidgetPtr) {
        qDebug() << "GetMediaWidget: Invalid MediaWidgetPtr.";
        return nullptr;
    }

    return m_mediaWidgetPtr;
}

bool QVideoPlayer::isMediaFileSupported(QString const &filePath) const
{
    bool bIsSupported = false;
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);

    // Gifs are supported but don't have a valid mime type
    if (mimeType.suffixes().contains("gif")) {
        bIsSupported = true;
    }

    // Check that mime types are videos
    QList<QString> mimeTypeAliases = mimeType.aliases();
    for (const QString &mimeType : mimeTypeAliases) {
        if (mimeType.startsWith("video/", Qt::CaseInsensitive)) {
            bIsSupported = true;
        }
    }

    qDebug() << mimeTypeAliases;
    return bIsSupported;
}

void QVideoPlayer::setMediaFile(QString const Path)
{
    if (m_mediaPlayerPtr) {
        if (m_mediaPlayerPtr->isPlaying() || m_mediaPlayerPtr->mediaStatus() != QMediaPlayer::NoMedia) {
            m_mediaPlayerPtr->stop();
        }
        qDebug() << "SetMediaFile: Setting new media file.";

        connect(m_mediaPlayerPtr, &QMediaPlayer::mediaStatusChanged, this, &QVideoPlayer::onMediaStatusChanged_Internal);
        m_mediaPlayerPtr->setSource(QUrl::fromLocalFile(Path));

        QMediaInfo NewMediaInfo{Path,
                                m_mediaPlayerPtr->duration(),
                                m_mediaPlayerPtr->metaData().value(QMediaMetaData::VideoFrameRate).toFloat(),
                                m_mediaPlayerPtr->metaData().value(QMediaMetaData::Resolution).toSize()};
        m_activeMediaInfo = NewMediaInfo;

    } else {
        qDebug() << "SetMediaFile: Invalid MediaPlayerPtr.";
    }
}

bool QVideoPlayer::getIsLooping()
{
    return m_bIsLooping;
}

void QVideoPlayer::play()
{
    if (m_mediaPlayerPtr) {
        qDebug() << "Play: Playing video.";
        m_mediaPlayerPtr->play();
    } else {
        qDebug() << "Play: Invalid MediaPlayerPtr.";
    }
}

void QVideoPlayer::pause()
{
    if (m_mediaPlayerPtr) {
        qDebug() << "Pause: Paused video.";
        m_mediaPlayerPtr->pause();
    } else {
        qDebug() << "Pause: Invalid MediaPlayerPtr.";
    }
}

void QVideoPlayer::setCurrentPosition(unsigned int const NewPosition)
{
    if (m_mediaPlayerPtr) {
        m_mediaPlayerPtr->setPosition(NewPosition);
    } else {
        qDebug() << "SetCurrentPosition: Invalid MediaPlayerPtr.";
    }
}

void QVideoPlayer::setIsLooping(bool const bIsLooping)
{
    m_bIsLooping = bIsLooping;
}

void QVideoPlayer::setLoopStartEnd(unsigned int const loopStart, unsigned int const loopEnd)
{
    m_loopStart = fmin(loopStart, loopEnd - MIN_LOOP_DURATION);
    m_loopEnd = fmax(loopEnd, m_loopStart + MIN_LOOP_DURATION);
}

unsigned int QVideoPlayer::getCurrentPosition()
{
    unsigned int CurrentPosition = 0.F;
    if (m_mediaPlayerPtr) {
        CurrentPosition = m_mediaPlayerPtr->position();
    } else {
        qDebug() << "GetCurrentPosition: Invalid MediaPlayerPtr.";
    }

    return CurrentPosition;
}

QMediaInfo *const QVideoPlayer::getMediaInfo()
{
    return &m_activeMediaInfo;
}

bool QVideoPlayer::getIsPlaying()
{
    bool bIsPlaying = false;
    if (m_mediaPlayerPtr) {
        bIsPlaying = m_mediaPlayerPtr->isPlaying();
    } else {
        qDebug() << "GetIsPlaying: Invalid MediaPlayerPtr.";
    }

    return bIsPlaying;
}
