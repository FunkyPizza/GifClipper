#ifndef TRANSCODERFACTORY_H
#define TRANSCODERFACTORY_H

#include "TranscoderInterface.h"
#include "Transcoders/GifCmdTranscoder.h"
#include "Transcoders/GifTranscoder.h"
#include "Transcoders/MP4CmdTranscoder.h"
#include "Transcoders/MP4Transcoder.h"

enum class TranscoderFormat { GIF, GIF_CMD, MP4, MP4_CMD };

class TranscoderFactory
{
public:
    static TranscoderInterface *createTranscoder(TranscoderFormat format)
    {
        switch (format) {
        case TranscoderFormat::GIF:
            return new GifTranscoder();
        case TranscoderFormat::GIF_CMD:
            return new GifCmdTranscoder();
        case TranscoderFormat::MP4:
            return new MP4Transcoder();
        case TranscoderFormat::MP4_CMD:
            return new MP4CmdTranscoder();
        default:
            return nullptr;
        }
    }  
};

#endif // TRANSCODERFACTORY_H
