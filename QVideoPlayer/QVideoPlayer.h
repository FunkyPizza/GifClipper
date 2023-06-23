#ifndef QVIDEOPLAYER_H
#define QVIDEOPLAYER_H

#include "QMediaPlayer.h"
#include "qsize.h"
#include <QObject>

class QVideoPlayerWidget;

struct QMediaInfo
{
    QString filename = ""; // Absolute path
    qint64 duration = 0;   // Duration in ms
    float framerate = 0;
    QSize resolution = QSize();
};

class QVideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit QVideoPlayer(QObject *parent = nullptr);
    ~QVideoPlayer();

    // Player Controls
    void play();
    void pause();
    void setCurrentPosition(unsigned int const NewPosition);

    // Loop controls
    void setIsLooping(bool const bIsLooping);
    void setLoopStartEnd(unsigned int const loopStart, unsigned int const loopEnd);

    // Media file
    bool isMediaFileSupported(QString const &filePath) const;
    void setMediaFile(QString const Path);

    // Player information
    QMediaInfo *const getMediaInfo();
    unsigned int getCurrentPosition();
    bool getIsLooping();
    bool getIsPlaying();

    //Other
    QVideoPlayerWidget *const getMediaWidget();

signals:
    void onCurrentPositionChanged(unsigned int const CurrentPosition);
    void onPlayEnded();
    void onPlayerReadyToPlay();

private:
    /* Does some necessary setup before any media can be loaded. */
    void initialiseMediaPlayer();
    void onCurrentPositionChanged_Internal(unsigned int const CurrentPosition);
    void onMediaStatusChanged_Internal(QMediaPlayer::MediaStatus status);

    QMediaInfo m_activeMediaInfo;
    QMediaPlayer *m_mediaPlayerPtr = nullptr;
    QVideoPlayerWidget *m_mediaWidgetPtr = nullptr;

    bool m_bIsLooping = false;
    unsigned int m_loopStart = 0;
    unsigned int m_loopEnd = 0;

    static constexpr unsigned int MIN_LOOP_DURATION = 10;
};

#endif // QVIDEOPLAYER_H
