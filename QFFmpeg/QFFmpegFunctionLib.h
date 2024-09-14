#ifndef QFFMPEGFUNCTIONLIB_H
#define QFFMPEGFUNCTIONLIB_H

#include <QString>

class QSize;
class QPoint;

enum VideoResolution { R_320, R_640, R_1280, R_1920 };
enum VideoFrameRate { F_5, F_10, F_15, F_20, F_30 };
enum VideoQuality { Q_Low = 0, Q_Normal = 1, Q_High = 2 };

class QFFmpegFunctionLib
{
public:
    QFFmpegFunctionLib();

    bool trimVideoToGif(const QString &inputFilename,
                        const QString &outputFilename,
                        float startTrim,
                        float endTrim,
                        QSize cropSize,
                        QPoint cropTopLeft,
                        int Resolution,
                        float FrameRate,
                        int Quality);

    bool trimVideoToMP4(const QString &inputFilename,
                        const QString &outputFilename,
                        float startTrim,
                        float endTrim,
                        QSize cropSize,
                        QPoint cropTopLeft,
                        int Resolution,
                        float FrameRate,
                        int Quality);

    static float VideoResolutionAsFloat(VideoResolution Resolution)
    {
        float outResolution = 320;
        switch (Resolution) {
        case VideoResolution::R_320:
            outResolution = 320;
            break;
        case VideoResolution::R_640:
            outResolution = 640;
            break;
        case VideoResolution::R_1280:
            outResolution = 1280;
            break;
        case VideoResolution::R_1920:
            outResolution = 1920;
            break;
        }

        return outResolution;
    };
    static float VideoFrameRateAsFloat(VideoFrameRate FrameRate)
    {
        float outFrameRate = 5;
        switch (FrameRate) {
        case VideoFrameRate::F_5:
            outFrameRate = 5;
            break;
        case VideoFrameRate::F_10:
            outFrameRate = 10;
            break;
        case VideoFrameRate::F_15:
            outFrameRate = 15;
            break;
        case VideoFrameRate::F_20:
            outFrameRate = 20;
            break;
        case VideoFrameRate::F_30:
            outFrameRate = 30;
            break;
        }

        return outFrameRate;
    };
    static QString VideoQualityAsString(VideoQuality Quality)
    {
        QString outString;
        switch (Quality) {
        case VideoQuality::Q_Low:
            outString = QString("Low");
            break;
        case VideoQuality::Q_Normal:
            outString = QString("Normal");
            break;
        case VideoQuality::Q_High:
            outString = QString("High");
            break;
        }

        return outString;
    }
    static QString VideoQualityToVideoCRFString(VideoQuality Quality)
    {
        QString outString;
        switch (Quality) {
        case VideoQuality::Q_Low:
            outString = QString("28");
            break;
        case VideoQuality::Q_Normal:
            outString = QString("23");
            break;
        case VideoQuality::Q_High:
            outString = QString("20");
            break;
        }

        return outString;
    }
    static QString VideoQualityToVideoPresetString(VideoQuality Quality)
    {
        QString outString;
        switch (Quality) {
        case VideoQuality::Q_Low:
            outString = QString("veryfast");
            break;
        case VideoQuality::Q_Normal:
            outString = QString("medium");
            break;
        case VideoQuality::Q_High:
            outString = QString("slow");
            break;
        }

        return outString;
    }
    static QString VideoQualityToGifDitherString(VideoQuality Quality)
    {
        QString outString;
        switch (Quality) {
        case VideoQuality::Q_Low:
            outString = QString("dither=none");
            break;
        case VideoQuality::Q_Normal:
            outString = QString("dither=bayer:bayer_scale=5");
            break;
        case VideoQuality::Q_High:
            outString = QString("dither=bayer:bayer_scale=3");
            break;
        }

        return outString;
    }
    static QString VideoQualityToGifPaletteGenString(VideoQuality Quality)
    {
        QString outString;
        switch (Quality) {
        case VideoQuality::Q_Low:
            outString = QString("max_colors=128");
            break;
        case VideoQuality::Q_Normal:
            outString = QString("max_colors=128");
            break;
        case VideoQuality::Q_High:
            outString = QString("max_colors=256");
            break;
        }

        return outString;
    }
};

#endif // QFFMPEGFUNCTIONLIB_H
