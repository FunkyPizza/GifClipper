#include "QFFmpegFunctionLib.h"
#include "qpoint.h"
#include "qsize.h"
#include <QDebug>
#include <QProcess>
#include <QString>
#include <cstdlib>
#include <direct.h>
#include <fstream>
#include <iostream>

extern "C" {
#include "libavformat/avio.h"
#include "libavutil/common.h"
#include "libavutil/dict.h"
#include "libavutil/log.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <stdint.h>
}

QFFmpegFunctionLib::QFFmpegFunctionLib() {}

bool QFFmpegFunctionLib::trimVideoToGif(const QString &inputFilename,
                                        const QString &outputFilename,
                                        float startTrim,
                                        float endTrim,
                                        QSize cropSize,
                                        QPoint cropTopLeft,
                                        int Resolution,
                                        float FrameRate)
{
    // Command string
    std::string outputResolution = std::to_string(Resolution);
    std::string outputFrameRate = std::to_string(FrameRate);

    std::string cropWidth = std::to_string(cropSize.width());
    std::string cropHeight = std::to_string(cropSize.height());
    std::string cropX = std::to_string(cropTopLeft.x());
    std::string cropY = std::to_string(cropTopLeft.y());

    char cwd[256];
    getcwd(cwd, 256);
    std::string ffmpegPath = "\"" + std::string(cwd) + "\\ffmpeg.exe\"";

    std::string commandConvert = ffmpegPath + " -y";                // ffmpeg exe
    commandConvert += " -i \"" + inputFilename.toUtf8() + "\"";     // input file
    commandConvert += " -ss " + std::to_string(startTrim);          // start trim
    commandConvert += " -t " + std::to_string(endTrim - startTrim); // duration
    commandConvert += " -filter_complex \"";                        // used for palettegen
    commandConvert += "crop=" + cropWidth + ":" + cropHeight;       // crop size
    commandConvert += ":" + cropX + ":" + cropY;                    // crop top left position
    commandConvert += ",fps=" + outputFrameRate;                    // framerate
    commandConvert += ",scale=" + outputResolution;                 // resolution
    commandConvert += ":-1:flags=lanczos";                          // scale algo
    commandConvert += ",split[o1] [o2];[o1] palettegen[p]; [o2] fifo [o3];[o3] [p] paletteuse\"";
    commandConvert += " \"" + outputFilename.toUtf8() + "\""; // output file

    qDebug().noquote() << "Video conversion starting.";
    qDebug().noquote() << "FFMPEG Path: " + ffmpegPath;
    qDebug().noquote() << "Input: " + inputFilename;
    qDebug().noquote() << "Output: " + outputFilename;
    qDebug().noquote() << "Trim: " + std::to_string(startTrim) + " to: " + std::to_string(endTrim);
    qDebug().noquote() << "Resolution: " + outputResolution;
    qDebug().noquote() << "FrameRate: " + outputFrameRate;
    qDebug().noquote() << "Command: " + commandConvert;

    // Execute the command
    //int ret = system(commandConvert.c_str()); //Escape characters don't get escaped?=
    int ret = QProcess::execute(QString::fromStdString(commandConvert));

    if (ret != 0) {
        qDebug() << "FFMPEG command failed.";
        return false;
    }

    qDebug() << "FFMPEG command success!";
    return true;
}
