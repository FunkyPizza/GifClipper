#ifndef QFFMPEGFUNCTIONLIB_H
#define QFFMPEGFUNCTIONLIB_H

#include <QString>

class QSize;
class QPoint;

enum VideoResolution { R_320, R_640, R_1280, R_1920 };
enum VideoFrameRate { F_5, F_10, F_15, F_20, F_30 };

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
                        float FrameRate);

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
};

#endif // QFFMPEGFUNCTIONLIB_H
