#include "GifTranscoder.h"
#include <QDebug>
#include "TranscoderQualityStatics.h"
#include <chrono>
#include <cstdio>

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

// Constructor
GifTranscoder::GifTranscoder()
    : m_inputFormatContext(nullptr)
    , m_outputFormatContext(nullptr)
    , m_decoderContext(nullptr)
    , m_encoderContext(nullptr)
    , m_filterGraph(nullptr)
    , m_bufferSourceContext(nullptr)
    , m_bufferSinkContext(nullptr)
    , m_paletteSourceContext(nullptr)
    , m_paletteFrame(nullptr)
{}

// Destructor
GifTranscoder::~GifTranscoder()
{
    cleanup();
}

// Initialize the encoder with provided parameters
bool GifTranscoder::initialize(const std::string &inputFile,
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

    // Step 1: Creating inputFormatContext & decoderContext
    // Open input file
    if (avformat_open_input(&m_inputFormatContext, m_inputFile.c_str(), nullptr, nullptr) < 0) {
        qCritical() << "Could not open input file:" << QString::fromStdString(m_inputFile);
        return false;
    }

    // Retrieve input file steam info
    if (avformat_find_stream_info(m_inputFormatContext, nullptr) < 0) {
        qCritical() << "Could not find stream information in:" << QString::fromStdString(m_inputFile);
        return false;
    }

    // Find the first stream we have a decoder for
    const AVCodec* decoder = nullptr;
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < m_inputFormatContext->nb_streams; ++i) {
        if (m_inputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            decoder = avcodec_find_decoder(m_inputFormatContext->streams[i]->codecpar->codec_id);
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qCritical() << "Could not find a video stream in the input file:" << QString::fromStdString(m_inputFile);
        return false;
    }

    if (!decoder) {
        qCritical() << "Unsupported codec in input file:" << QString::fromStdString(m_inputFile);
        return false;
    }

    // Set up decoder context
    m_decoderContext = avcodec_alloc_context3(decoder);
    if (!m_decoderContext) {
        qCritical() << "Could not allocate decoder context.";
        return false;
    }

    if (avcodec_parameters_to_context(m_decoderContext, m_inputFormatContext->streams[videoStreamIndex]->codecpar) < 0) {
        qCritical() << "Could not copy codec parameters to decoder context.";
        return false;
    }

    if (avcodec_open2(m_decoderContext, decoder, nullptr) < 0) {
        qCritical() << "Could not open decoder.";
        return false;
    }

    // Step 2: Creating outputFormatContext & encoderContext
    // Allocate output format context
    if (avformat_alloc_output_context2(&m_outputFormatContext, nullptr, nullptr, m_outputFile.c_str()) < 0) {
        qCritical() << "Could not create output context for:" << QString::fromStdString(m_outputFile);
        return false;
    }

    // Find GIF encoder
    const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_GIF);
    if (!encoder) {
        qCritical() << "GIF encoder not found.";
        return false;
    }

    // Create new video stream in output
    AVStream* outStream = avformat_new_stream(m_outputFormatContext, nullptr);
    if (!outStream) {
        qCritical() << "Failed allocating output stream.";
        return false;
    }

    // Allocate encoder context
    m_encoderContext = avcodec_alloc_context3(encoder);
    if (!m_encoderContext) {
        qCritical() << "Could not allocate encoder context.";
        return false;
    }

    // Step 3: Generate the palette frame (first pass filter)
    if (!generatePalette()) {
        qCritical() << "Failed to generate palette.";
        return false;
    }

    // Step 4: Set up the filter pass (second pass fiter)
    if (!setupFilters()) {
        qCritical() << "Failed to set up filters.";
        return false;
    }

    // Configure the filter graph
    if (avfilter_graph_config(m_filterGraph, nullptr) < 0) {
        qCritical() << "Error configuring filter graph.";
        return false;
    }

    // Step 5: Configure encoder from filter
    // Get output dimensions from the filter graph
    AVFilterContext* lastFilter = m_bufferSinkContext;
    m_encoderContext->width = av_buffersink_get_w(lastFilter);
    m_encoderContext->height = av_buffersink_get_h(lastFilter);

    // Set encoder parameters
    m_encoderContext->sample_aspect_ratio = m_decoderContext->sample_aspect_ratio;
    m_encoderContext->pix_fmt = AV_PIX_FMT_PAL8; // GIF uses palette-based colors
    m_encoderContext->time_base = {1, m_fps};
    m_encoderContext->codec_type = AVMEDIA_TYPE_VIDEO;
    m_encoderContext->codec_id = AV_CODEC_ID_GIF;

    // Encoding parameters
    m_encoderContext->bit_rate = 1000000; // Adjust as needed
    m_encoderContext->gop_size = 0;       // No GOP for GIF
    m_encoderContext->max_b_frames = 0;

    // Open encoder
    if (avcodec_open2(m_encoderContext, encoder, nullptr) < 0) {
        qCritical() << "Cannot open GIF encoder.";
        return false;
    }

    // Copy encoder parameters to output stream
    if (avcodec_parameters_from_context(outStream->codecpar, m_encoderContext) < 0) {
        qCritical() << "Failed to copy encoder parameters to output stream.";
        return false;
    }

    outStream->time_base = m_encoderContext->time_base;

    // Open output file
    if (!(m_outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_outputFormatContext->pb, m_outputFile.c_str(), AVIO_FLAG_WRITE) < 0) {
            qCritical() << "Could not open output file:" << QString::fromStdString(m_outputFile);
            return false;
        }
    }

    // Write the stream header
    if (avformat_write_header(m_outputFormatContext, nullptr) < 0) {
        qCritical() << "Error occurred when writing header to output file.";
        return false;
    }

    return true;
}

// Generate the palette (First Pass)
bool GifTranscoder::generatePalette()
{
    // Here we are creating a full filter graph that will only be used in the scope of palette generation (ie: this func)
    int ret = 0;
    AVFilterInOut* outputs = nullptr;
    AVFilterInOut* inputs = nullptr;
    AVFilterGraph* paletteFilterGraph = avfilter_graph_alloc();
    AVFilterContext* paletteBuffersrcCtx = nullptr;
    AVFilterContext* paletteBuffersinkCtx = nullptr;
    AVRational time_base = m_inputFormatContext->streams[0]->time_base;
    AVCodecParameters *codecpar = m_inputFormatContext->streams[0]->codecpar;

    if (!paletteFilterGraph) {
        qCritical() << "Unable to create palette filter graph.";
        return false;
    }

    // Step 1: Construct the filter graph
    char args[512];
    snprintf(args,
             sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             codecpar->width,
             codecpar->height,
             codecpar->format,
             time_base.num,
             time_base.den,
             codecpar->sample_aspect_ratio.num,
             codecpar->sample_aspect_ratio.den);

    // Create buffer source (filter input)
    ret = avfilter_graph_create_filter(&paletteBuffersrcCtx, avfilter_get_by_name("buffer"), "in", args, nullptr, paletteFilterGraph);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Cannot create buffer source for palette generation:" << errbuf;
        return false;
    }

    // Create buffer sink (filter output)
    ret = avfilter_graph_create_filter(&paletteBuffersinkCtx, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, paletteFilterGraph);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Cannot create buffer sink for palette generation:" << errbuf;
        return false;
    }

    char filter_descr[512];
    std::string pallettegenArgs = TranscoderQualityStatics::getPalettegenArgsFromQualityLevel(m_qualityLevel);
    snprintf(filter_descr,
             sizeof(filter_descr),
             "crop=%d:%d:%d:%d,fps=%d,scale=%d:-2:flags=lanczos,palettegen%s",
             m_cropW,
             m_cropH,
             m_cropX,
             m_cropY,
             m_fps,
             m_width,
             pallettegenArgs.c_str());

    qDebug() << "Palette generation filter description:" << filter_descr;

    // Parse the filter graph
    ret = avfilter_graph_parse_ptr(paletteFilterGraph, filter_descr, &inputs, &outputs, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Error parsing palette generation filter graph:" << errbuf;
        return false;
    }

    // Link the filters
    ret = avfilter_link(paletteBuffersrcCtx, 0, inputs->filter_ctx, inputs->pad_idx);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Error linking buffer source to palette generation filter graph:" << errbuf;
        return false;
    }

    ret = avfilter_link(outputs->filter_ctx, outputs->pad_idx, paletteBuffersinkCtx, 0);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Error linking palette generation filter graph to buffer sink:" << errbuf;
        return false;
    }

    // Free the in/out structures
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    // Configure the filter graph
    ret = avfilter_graph_config(paletteFilterGraph, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Error configuring palette generation filter graph:" << errbuf;
        return false;
    }

    // Step 2: Send all frames through to the filter graph
    AVPacket *packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* filt_frame = av_frame_alloc();

    if (!packet || !frame || !filt_frame) {
        qCritical() << "Could not allocate packet or frames for palette generation.";
        return false;
    }

    // Seek to start time
    int64_t seek_target = static_cast<int64_t>(m_startTime / av_q2d(AV_TIME_BASE_Q));
    if (av_seek_frame(m_inputFormatContext, -1, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
        qWarning() << "Error seeking to start time for palette generation.";
    }
    avcodec_flush_buffers(m_decoderContext);

    bool canStartNextFrame = true;
    bool endReached = false;
    while (!endReached) {
        // Step A: If no frame is currently processing, get next frame and send it to the decoder
        if (canStartNextFrame) {
            int retVal = av_read_frame(m_inputFormatContext, packet);
            if (retVal >= 0) {
                // Check if the frame belongs to the video stream
                if (packet->stream_index == 0) {
                    // Send frame packet to the decoder
                    if (avcodec_send_packet(m_decoderContext, packet) < 0) {
                        qWarning() << "Error sending packet to decoder.";
                        av_packet_unref(packet); // Free the packet
                        break;
                    }
                    canStartNextFrame = false;
                    av_packet_unref(packet); // Unref packet after sending
                }
            } else if (retVal == AVERROR_EOF) {
                // No more frames/packet to read.
                endReached = true;
                av_packet_unref(packet);
            } else {
                qWarning() << "Error reading from input while generating palette.";
                av_packet_unref(packet);
                endReached = true;
                break;
            }
        }

        // Step B: Send decoded frames into the palette filter graph
        int retFilter;
        while ((retFilter = avcodec_receive_frame(m_decoderContext, frame)) == 0) {
            // If PTS is not set, use the best effort timestamp
            if (frame->pts == AV_NOPTS_VALUE) {
                frame->pts = frame->best_effort_timestamp;
            }
            double frameTime = frame->pts * av_q2d(m_inputFormatContext->streams[0]->time_base);
            qDebug() << "Decoded palette frame PTS:" << frame->pts << " Time:" << frameTime;

            // Check if the frame time exceeds endTrim
            if (frameTime > m_endTime) {
                qDebug() << "Reached end of trimmed section at frameTime:" << frameTime;
                endReached = true; // Exit the processing loop
            }

            // Push the decoded frame into the filter graph
            if (av_buffersrc_add_frame_flags(paletteBuffersrcCtx, frame, AV_BUFFERSRC_FLAG_KEEP_REF)
                < 0) {
                qWarning() << "Error while feeding the filter graph during palette generation.";
                av_frame_unref(frame);
                endReached = true;
                break;
            }

            canStartNextFrame = true;
            av_frame_unref(frame);
        }
        // Check if frame is skipped by the decoder
        if (retFilter == AVERROR_EOF || retFilter == AVERROR(EAGAIN)) {
            canStartNextFrame = true;
        }
    }

    // Flush the filter graph
    av_buffersrc_add_frame_flags(paletteBuffersrcCtx, nullptr, AV_BUFFERSRC_FLAG_KEEP_REF);

    // Retrieve the palette frame from the filter graph
    while (av_buffersink_get_frame(paletteBuffersinkCtx, filt_frame) >= 0) {
        // Store the palette frame
        m_paletteFrame = av_frame_clone(filt_frame);
        av_frame_unref(filt_frame);
        break;
    }

    if (!m_paletteFrame) {
        qCritical() << "Palette was not generated after processing all frames.";
        // Cleanup
        avfilter_graph_free(&paletteFilterGraph);
        av_frame_free(&frame);
        av_frame_free(&filt_frame);
        av_packet_free(&packet);
        return false;
    }

    // Cleanup
    avfilter_graph_free(&paletteFilterGraph);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    av_packet_free(&packet);

    return true;
}

// Set up the filter graph (Second Pass)
bool GifTranscoder::setupFilters()
{
    int ret;
    char args[512];
    AVFilterInOut* inputs = nullptr;
    AVFilterInOut* outputs = nullptr;

    m_filterGraph = avfilter_graph_alloc();
    if (!m_filterGraph) {
        qCritical() << "Unable to create filter graph.";
        return false;
    }

    // Step 1: Create bufferSourceContext (ie: the input for this filter)
    AVCodecParameters* codecpar = m_inputFormatContext->streams[0]->codecpar;
    AVRational time_base = m_inputFormatContext->streams[0]->time_base;
    snprintf(args,
             sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             codecpar->width,
             codecpar->height,
             codecpar->format,
             time_base.num,
             time_base.den,
             codecpar->sample_aspect_ratio.num,
             codecpar->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&m_bufferSourceContext, avfilter_get_by_name("buffer"), "video_in", args, nullptr, m_filterGraph);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Cannot create video buffer source:" << errbuf;
        return false;
    }

    // Step 2: Create filter input for the palette frame (ie: the input for the palette frame)
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=1/25:pixel_aspect=1/1", m_paletteFrame->width, m_paletteFrame->height, m_paletteFrame->format);

    ret = avfilter_graph_create_filter(&m_paletteSourceContext, avfilter_get_by_name("buffer"), "palette_in", args, nullptr, m_filterGraph);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Cannot create palette buffer source:" << errbuf;
        return false;
    }

    // Step 3: Create bufferSinkContext (ie: the output of this filter)
    ret = avfilter_graph_create_filter(&m_bufferSinkContext, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, m_filterGraph);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Cannot create buffer sink:" << errbuf;
        return false;
    }

    // Step 4: Construct the filter graph
    char filter_descr[1024];
    std::string paletteuseArgs = TranscoderQualityStatics::getPaletteuseArgsFromQualityLevel(m_qualityLevel);
    snprintf(filter_descr,
             sizeof(filter_descr),
             "[video_in] crop=%d:%d:%d:%d, fps=%d, scale=%d:-2:flags=lanczos [filtered]; "
             "[filtered][palette_in] paletteuse%s [out]",
             m_cropW,
             m_cropH,
             m_cropX,
             m_cropY,
             m_fps,
             m_width,
             paletteuseArgs.c_str());

    qDebug() << "Filter description:" << filter_descr;

    // Parse the filter graph
    ret = avfilter_graph_parse_ptr(m_filterGraph, filter_descr, &inputs, &outputs, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "Error parsing filter graph:" << errbuf;
        return false;
    }

    // Inputs
    for (AVFilterInOut* curr = inputs; curr; curr = curr->next) {
        if (strcmp(curr->name, "video_in") == 0) {
            ret = avfilter_link(m_bufferSourceContext, 0, curr->filter_ctx, curr->pad_idx);
        } else if (strcmp(curr->name, "palette_in") == 0) {
            ret = avfilter_link(m_paletteSourceContext, 0, curr->filter_ctx, curr->pad_idx);
        } else {
            qCritical() << "Unknown input to filter graph:" << curr->name;
            return false;
        }
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qCritical() << "Error linking buffer source:" << errbuf;
            return false;
        }
    }

    // Outputs
    for (AVFilterInOut* curr = outputs; curr; curr = curr->next) {
        if (strcmp(curr->name, "out") == 0) {
            ret = avfilter_link(curr->filter_ctx, curr->pad_idx, m_bufferSinkContext, 0);
        } else {
            qCritical() << "Unknown output from filter graph:" << curr->name;
            return false;
        }
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qCritical() << "Error linking filter graph to buffer sink:" << errbuf;
            return false;
        }
    }

    // Free the in/out structures
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    // Checking the graoh configuration is correct
    ret = avfilter_graph_config(m_filterGraph, nullptr);
    if (ret < 0) {
        qCritical() << "Error configuring filter graph.";
        return false;
    }

    return true;
}

// Perform the encoding process
bool GifTranscoder::transcode()
{
    // Step 1: Init transcoding variables
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* filt_frame = av_frame_alloc();

    if (!packet || !frame || !filt_frame) {
        qCritical() << "Could not allocate packet or frames.";
        return false;
    }

    // Seek to start time
    int64_t seek_target = static_cast<int64_t>(m_startTime * AV_TIME_BASE);
    if (av_seek_frame(m_inputFormatContext, -1, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
        qWarning() << "Error seeking to start time.";
    }
    avcodec_flush_buffers(m_decoderContext);

    // Store the buffer source time base (set during filter creation)
    AVRational bufferSrcTimeBase = m_inputFormatContext->streams[0]->time_base;

    // Step 2: Iterate through frames transcoding them, frames are processed sequentially / one by one
    auto startTime = std::chrono::high_resolution_clock::now();
    bool canStartNextFrame = true;
    bool endReached = false;
    while (!endReached) {
        if (bAllowNonSequentialTranscoding) {
            // DEBUG ONLY: currently will stop when the last frame is sent to decoder (ie: output might miss some frames at the end)
            canStartNextFrame = true;
        }

        // Step A: If no frame is currently processing, get next frame and send it to the decoder
        if (canStartNextFrame) {
            int retVal = av_read_frame(m_inputFormatContext, packet);
            if (retVal >= 0) {
                // Check if the frame belongs to the video stream
                if (packet->stream_index == 0) {
                    // Send frame packet to the decoder
                    qDebug() << "Decoding frame PTS:" << packet->dts;
                    if (avcodec_send_packet(m_decoderContext, packet) < 0) {
                        qWarning() << "Error sending packet to decoder.";
                        av_packet_unref(packet); // Free the packet
                        break;
                    }

                    canStartNextFrame = false;
                    av_packet_unref(packet); // Unref packet after sending
                }
            } else if (retVal == AVERROR_EOF) {
                // No more frames/packet to read.
                endReached = true;
                av_packet_unref(packet);
            } else {
                qWarning() << "Error reading from input.";
                av_packet_unref(packet);
                endReached = true;
                break;
            }
        }

        // Step B: Try to get the frame from the decoder and send it to the filter graph
        int retFilter;
        while ((retFilter = avcodec_receive_frame(m_decoderContext, frame)) == 0) {
            // If PTS is not set, use the best effort timestamp
            if (frame->pts == AV_NOPTS_VALUE) {
                frame->pts = frame->best_effort_timestamp;
            }
            double frameTime = frame->pts * av_q2d(m_inputFormatContext->streams[0]->time_base);
            qDebug() << "Decoded frame PTS:" << frame->pts << " Time:" << frameTime;

            // Check if the frame time exceeds endTrim
            if (frameTime > m_endTime) {
                qDebug() << "Reached end of trimmed section at frameTime:" << frameTime;
                endReached = true; // Exit the processing loop
            }

            // Add the palette frame to the palette buffer source (could not get fifo filter to work fine)
            m_paletteFrame->pts = frame->pts;
            if (av_buffersrc_add_frame_flags(m_paletteSourceContext, m_paletteFrame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                qWarning() << "Error adding palette frame to palette buffer source.";
                av_frame_unref(frame);
                endReached = true;
                break;
            }

            // Push the decoded frame into the filter graph
            qDebug() << "Filtering frame with PTS:" << frame->pts;
            if (av_buffersrc_add_frame(m_bufferSourceContext, frame) < 0) {
                qWarning() << "Error while feeding the filter graph with frame PTS.";
                av_frame_unref(frame); // Unref frame if error occurs
                endReached = true;
                break;
            }

            av_frame_unref(frame); // Unref the frame after sending to filter
        }
        // Check if frame is skipped by the decoder
        if (retFilter == AVERROR_EOF || retFilter == AVERROR(EAGAIN)) {
            canStartNextFrame = true;
        }

        // Step C: Try to get the frame from the filter graph and send it to the encoder
        while (av_buffersink_get_frame(m_bufferSinkContext, filt_frame) >= 0) {
            if (filt_frame->pts == AV_NOPTS_VALUE) {
                qWarning() << "Filtered frame has no PTS.";
                break;
            } else {
                qDebug() << "Filtered frame with PTS:" << filt_frame->pts;
            }

            // Encode the frame
            qDebug() << "Encoding filtered frame with PTS:" << filt_frame->pts;
            if (avcodec_send_frame(m_encoderContext, filt_frame) < 0) {
                qWarning() << "Error sending frame to encoder.";
                av_frame_unref(filt_frame); // Unref filt_frame if error occurs
                endReached = true;
                break;
            }

            av_frame_unref(filt_frame); // Unref after sending to encoder
        }

        // Step D: Try to get the frame from the encoder and write it to file
        AVPacket* enc_pkt = av_packet_alloc();
        if (!enc_pkt) {
            qCritical() << "Could not allocate encoding packet.";
            return false;
        }

        while (avcodec_receive_packet(m_encoderContext, enc_pkt) == 0) {
            qDebug() << "Encoded packet received with PTS:" << enc_pkt->pts;

            // Rescale PTS/DTS
            AVStream* out_stream = m_outputFormatContext->streams[0];
            av_packet_rescale_ts(enc_pkt, m_encoderContext->time_base, out_stream->time_base);
            enc_pkt->stream_index = 0;

            // Write the encoded frame to the output file
            qDebug() << "Writing packet to output file with PTS:" << enc_pkt->pts;
            if (av_interleaved_write_frame(m_outputFormatContext, enc_pkt) < 0) {
                qWarning() << "Error writing frame to output file.";
                av_packet_free(&enc_pkt);
                endReached = true;
                break;
            }

            canStartNextFrame = true;
        }

        av_packet_free(&enc_pkt);
    }

    auto totalElapsed = std::chrono::high_resolution_clock::now() - startTime;
    double totalTimeMs = std::chrono::duration<double, std::milli>(totalElapsed).count();
    qDebug() << "Total encoding time:" << totalTimeMs << "ms";

    av_write_trailer(m_outputFormatContext);

    // Cleanup
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    av_packet_free(&packet);

    return true;
}

// Cleanup all allocated resources
void GifTranscoder::cleanup()
{
    if (m_paletteFrame) {
        av_frame_free(&m_paletteFrame);
    }
    if (m_filterGraph) {
        avfilter_graph_free(&m_filterGraph);
    }
    if (m_decoderContext) {
        avcodec_free_context(&m_decoderContext);
    }
    if (m_encoderContext) {
        avcodec_free_context(&m_encoderContext);
    }
    if (m_inputFormatContext) {
        avformat_close_input(&m_inputFormatContext);
    }
    if (m_outputFormatContext) {
        if (!(m_outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&m_outputFormatContext->pb);
        }
        avformat_free_context(m_outputFormatContext);
    }
}
