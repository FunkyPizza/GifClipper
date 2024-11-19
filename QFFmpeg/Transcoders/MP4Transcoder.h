#ifndef MP4TRANSCODER_H
#define MP4TRANSCODER_H

#include "TranscoderInterface.h"
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}

class MP4Transcoder : public TranscoderInterface
{
public:
    MP4Transcoder();
    ~MP4Transcoder();

    /* ==== Encoder Interface ==== */
    bool initialize(const std::string &inputFile,
                    const std::string &outputFile,
                    double startTime,
                    double endTime,
                    int cropW,
                    int cropH,
                    int cropX,
                    int cropY,
                    int fps,
                    int width,
                    int qualityLevel) override;
    bool transcode() override;

private:
    /* ==== Gif encoder ==== */
    bool setupFilters(); // Second pass: set up filters with paletteuse
    void cleanup();

    /* ==== Encoding members ==== */
    AVFormatContext *m_inputFormatContext;  // input file format stuff
    AVFormatContext *m_outputFormatContext; // mp4 format stuff
    AVCodecContext *m_decoderContext;       // decoding input file
    AVCodecContext *m_encoderContext;       // encoding into gif
    AVFilterGraph *m_filterGraph;           // filter to apply trim, crop, fps
    AVFilterContext *m_bufferSourceContext; // filter input
    AVFilterContext *m_bufferSinkContext;   // filter output

    /* ==== Encoding parameters ==== */
    std::string m_inputFile;  // input filename
    std::string m_outputFile; // output filename
    double m_startTime;       // trim start time
    double m_endTime;         // trim end time
    int m_cropW;              // crop area width
    int m_cropH;              // crop area height
    int m_cropX;              // crop area top left X position
    int m_cropY;              // crop area top left Y position
    int m_fps;                // target fps
    int m_width;              // target width resolution
    int m_qualityLevel;       // target quality level

    /* ==== Debug ==== */
    bool bAllowNonSequentialTranscoding = false; //will cause some dropped frames at the end
};

#endif // MP4TRANSCODER_H