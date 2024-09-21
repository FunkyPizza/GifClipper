#include "GifCmdTranscoder.h"
#include <QDebug>
#include <QProcess>
#include "TranscoderQualityStatics.h"
#include <chrono>
#include <cstdio>
#include <io.h>

// Constructor
GifCmdTranscoder::GifCmdTranscoder() {}

// Destructor
GifCmdTranscoder::~GifCmdTranscoder() {}

// Initialize the encoder with provided parameters
bool GifCmdTranscoder::initialize(const std::string &inputFile,
                                  const std::string &outputFile,
                                  double startTime,
                                  double endTime,
                                  int cropW,
                                  int cropH,
                                  int cropX,
                                  int cropY,
                                  int fps,
                                  int width,
                                  int qualityLevel)
{
    // Set up encoding parameters
    m_inputFile = inputFile;
    m_outputFile = outputFile;
    m_startTime = startTime;
    m_endTime = endTime;
    m_cropW = cropW;
    m_cropH = cropH;
    m_cropX = cropX;
    m_cropY = cropY;
    m_fps = fps;
    m_width = width;
    m_qualityLevel = qualityLevel;

    return true;
}

// Perform the encoding process (Second Pass)
bool GifCmdTranscoder::transcode()
{
    // Command string
    std::string outputResolution = std::to_string(m_width);
    std::string outputFrameRate = std::to_string(m_fps);
    std::string outputPaletteGenArgs = TranscoderQualityStatics::getPalettegenArgsFromQualityLevel(
        m_qualityLevel);
    std::string outputPaletteuseArgs = TranscoderQualityStatics::getPaletteuseArgsFromQualityLevel(
        m_qualityLevel);

    std::string cropWidth = std::to_string(m_cropW);
    std::string cropHeight = std::to_string(m_cropH);
    std::string cropX = std::to_string(m_cropX);
    std::string cropY = std::to_string(m_cropY);

    char cwd[256];
    getcwd(cwd, 256);
    std::string ffmpegPath = "\"" + std::string(cwd) + "\\ffmpeg.exe\"";

    std::string commandConvert = ffmpegPath + " -y";                    // ffmpeg exe
    commandConvert += " -i \"" + m_inputFile + "\"";                    // input file
    commandConvert += " -ss " + std::to_string(m_startTime);            // start trim
    commandConvert += " -t " + std::to_string(m_endTime - m_startTime); // duration
    commandConvert += " -filter_complex \"";                            // used for palettegen
    commandConvert += "crop=" + cropWidth + ":" + cropHeight;           // crop size
    commandConvert += ":" + cropX + ":" + cropY;                        // crop top left position
    commandConvert += ",fps=" + outputFrameRate;                        // framerate
    commandConvert += ",scale=" + outputResolution;                     // resolution
    commandConvert += ":-1:flags=lanczos";                              // scale algo
    commandConvert += ",split[o1] [o2];[o1] palettegen" + outputPaletteGenArgs + "[p];";
    commandConvert += " [o2] fifo [o3];[o3][p] paletteuse" + outputPaletteuseArgs + "\"";
    commandConvert += " \"" + m_outputFile + "\""; // output file

    qDebug().noquote() << "Video conversion starting.";
    qDebug().noquote() << "FFMPEG Path: " + ffmpegPath;
    qDebug().noquote() << "Input: " + m_inputFile;
    qDebug().noquote() << "Output: " + m_outputFile;
    qDebug().noquote() << "Trim: " + std::to_string(m_startTime)
                              + " to: " + std::to_string(m_endTime);
    qDebug().noquote() << "Resolution: " + outputResolution;
    qDebug().noquote() << "FrameRate: " + outputFrameRate;
    qDebug().noquote() << "Command: " + commandConvert;

    // Execute the command
    auto startTime = std::chrono::high_resolution_clock::now();
    int ret = QProcess::execute(QString::fromStdString(commandConvert));

    auto totalElapsed = std::chrono::high_resolution_clock::now() - startTime;
    double totalTimeMs = std::chrono::duration<double, std::milli>(totalElapsed).count();
    qDebug() << "Total encoding time:" << totalTimeMs << "ms";

    if (ret != 0) {
        qDebug() << "FFMPEG command failed.";
        return false;
    }

    qDebug() << "FFMPEG command success!";

    return true;
}
