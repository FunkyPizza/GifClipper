#ifndef MP4CMDTRANSCODER_H
#define MP4CMDTRANSCODER_H

#include "TranscoderInterface.h"
#include <string>

class MP4CmdTranscoder : public TranscoderInterface
{
public:
    MP4CmdTranscoder();
    ~MP4CmdTranscoder();

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
};

#endif // MP4CMDTRANSCODER_H
