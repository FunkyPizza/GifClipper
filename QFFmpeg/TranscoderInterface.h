#ifndef TRANSCODERINTERFACE_H
#define TRANSCODERINTERFACE_H

#include <string>

class TranscoderInterface
{
public:
    virtual ~TranscoderInterface() = default;

    /* Initialize the transcoder
     * @param inputFile     Filepath to the input video file
     * @param outputFile    Filepath for the output video file
     * @param startTime     Start time of the trim selection
     * @param endTime       End time of the trim selection
     * @param cropW         Width of the cropped area
     * @param cropH         Height of the cropped area
     * @param cropX         X position of the Top Left corner of the crop area
     * @param cropY         Y position of the Top Left corner of the crop area
     * @param fps           Target FPS of the output video file
     * @param width         Target width of the output video file
     * @param qualityLevel  Quality of the transcoding (0=Low, 1=Normal, 2=High) 
     * @return              True if initialization was a success      
     */
    virtual bool initialize(const std::string &inputFile,
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
        = 0;

    /* Executes the transcoding work. 
     * @return      True if the transcoding was successful
     */
    virtual bool transcode() = 0;
};

#endif // TRANSCODERINTERFACE_H
