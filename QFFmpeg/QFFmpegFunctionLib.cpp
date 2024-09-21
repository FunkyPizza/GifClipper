#include "QFFmpegFunctionLib.h"
#include "TranscoderFactory.h"
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
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

QFFmpegFunctionLib::QFFmpegFunctionLib() {}

void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    if (level <= AV_LOG_INFO) {
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, vargs);
        qDebug() << "FFmpeg:" << buffer;
    }
}

bool QFFmpegFunctionLib::transcodeFile(const TranscoderFormat OutputFormat,
                                       const QString &inputFilename,
                                       const QString &outputFilename,
                                       float startTrim,
                                       float endTrim,
                                       QSize cropSize,
                                       QPoint cropTopLeft,
                                       int Resolution,
                                       float FrameRate,
                                       int Quality)
{
    av_log_set_level(AV_LOG_TRACE);
    av_log_set_callback(ffmpeg_log_callback);

    TranscoderInterface *transcoder = TranscoderFactory::createTranscoder(OutputFormat);
    if (!transcoder) {
        qDebug() << "Unsupported format.";
        return false;
    }

    bool initialized = transcoder->initialize(inputFilename.toStdString(),
                                              outputFilename.toStdString(),
                                              startTrim,
                                              endTrim,
                                              cropSize.width(),
                                              cropSize.height(),
                                              cropTopLeft.x(),
                                              cropTopLeft.y(),
                                              FrameRate,
                                              Resolution,
                                              Quality);

    if (!initialized) {
        qDebug() << "Failed to initialize transcoder.";
        delete transcoder;
        return false;
    }

    bool success = transcoder->transcode();
    if (!success) {
        qDebug() << "Transcoding failed.";
    }

    delete transcoder;
    return success;
}
