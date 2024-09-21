#ifndef TRANSCODERQUALITYSTATICS_H
#define TRANSCODERQUALITYSTATICS_H

#include <string>

class TranscoderQualityStatics
{
public:
    TranscoderQualityStatics();

    /* ==== FORMAT: GIF ==== */
    static std::string getPalettegenArgsFromQualityLevel(int qualityLevel)
    {
        std::string outString;
        switch (qualityLevel) {
        case 0: // Low quality
            outString = "=max_colors=64";
            break;
        case 1: // Medium quality
            outString = "=max_colors=128";
            break;
        case 2: // High quality
            outString = "=max_colors=256";
            break;
        }

        return outString;
    }
    static std::string getPaletteuseArgsFromQualityLevel(int qualityLevel)
    {
        std::string outString;
        switch (qualityLevel) {
        case 0: // Low quality
            outString = "=dither=none";
            break;
        case 1: // Medium quality
            outString = "=dither=bayer:bayer_scale=5";
            break;
        case 2: // High quality
            outString = "=dither=bayer:bayer_scale=3";
            break;
        }

        return outString;
    }

    /* ==== FORMAT: MP4 ==== */
    static std::string getCRFArgsFromQualityLevel(int qualityLevel)
    {
        std::string outString;
        switch (qualityLevel) {
        case 0:
            outString = "28";
            break;
        case 1:
            outString = "23";
            break;
        case 2:
            outString = "20";
            break;
        }

        return outString;
    }
    static std::string getCompressionPresetFromQualityLevel(int qualityLevel)
    {
        std::string outString;
        switch (qualityLevel) {
        case 0:
            outString = "veryfast";
            break;
        case 1:
            outString = "medium";
            break;
        case 2:
            outString = "slow";
            break;
        }
        return outString;
    };
};
#endif // TRANSCODERQUALITYSTATICS_H
