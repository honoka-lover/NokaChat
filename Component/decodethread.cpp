#include "decodethread.h"
#include <QDebug>
#include <QTime>
#include <QMediaDevices>
DecodeThread::DecodeThread(QObject* parent)
    : QThread(parent),
    pFormatContext(nullptr),
    audioCodecContext(nullptr),
    videoWidget(nullptr),
    stop(false),
    videoStreamIndex(-1),
    audioStreamIndex(-1),
    audioClock(0.0)
{

}

DecodeThread::~DecodeThread() {
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
    }
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
    }
    if (pFormatContext) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
    }

    mutex.lock();
    stop = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

void DecodeThread::setFormatContext(AVFormatContext* formatContext) {
    QMutexLocker locker(&mutex);
    pFormatContext = formatContext;
    videoStreamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStreamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    if (videoStreamIndex >= 0) {
        AVCodecParameters* codecParameters = pFormatContext->streams[videoStreamIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
        videoCodecContext = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(videoCodecContext, codecParameters);
        avcodec_open2(videoCodecContext, codec, nullptr);
    }

    if (audioStreamIndex >= 0) {
        AVCodecParameters* audioCodecParameters = pFormatContext->streams[audioStreamIndex]->codecpar;
        const AVCodec* audioCodec = avcodec_find_decoder(audioCodecParameters->codec_id);
        audioCodecContext = avcodec_alloc_context3(audioCodec);
        avcodec_parameters_to_context(audioCodecContext, audioCodecParameters);
        avcodec_open2(audioCodecContext, audioCodec, nullptr);       

    }
    condition.wakeOne();
}

void DecodeThread::setVideoWidget(VideoWidget* widget) {
    QMutexLocker locker(&mutex);
    videoWidget = widget;
    condition.wakeOne();
}

void DecodeThread::seek(int64_t timestamp) {
    QMutexLocker locker(&mutex);
    if (pFormatContext) {
        av_seek_frame(pFormatContext, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(videoCodecContext);
        avcodec_flush_buffers(audioCodecContext);
        condition.wakeOne();
    }
}

AVCodecContext *DecodeThread::getAudioCodecContext()
{
    return this->audioCodecContext;
}

void DecodeThread::run() {
    while (!stop) {
        mutex.lock();
        if (!pFormatContext) {
            condition.wait(&mutex);
        }
        AVPacket* pPacket = av_packet_alloc();
        AVFrame* pFrame = av_frame_alloc();
        if (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == videoStreamIndex) {
                if (avcodec_send_packet(videoCodecContext, pPacket) >= 0) {
                    while (avcodec_receive_frame(videoCodecContext, pFrame) >= 0) {
                        double pts = pFrame->best_effort_timestamp * av_q2d(pFormatContext->streams[videoStreamIndex]->time_base);
                        double delay = pts - audioClock;
                        if (delay > 0) {
                            msleep(static_cast<unsigned long>(delay));
                        }

                        QSharedPointer<AVFrame> frameCopy(av_frame_clone(pFrame), [](AVFrame* frame){
                            av_frame_free(&frame);
                        });
                        if (!frameCopy.isNull()) {
                            emit frameDecoded(frameCopy);
                        }

                    }
                }
            }else if (pPacket->stream_index == audioStreamIndex) {
                if (avcodec_send_packet(audioCodecContext, pPacket) >= 0) {
                    while (avcodec_receive_frame(audioCodecContext, pFrame) >= 0) {
                        // 转换音频数据为 PCM 并写入 audioIODevice
                        uint8_t* outputBuffer = nullptr;
                        int result = convertAudioFormat(pFrame, audioCodecContext, &outputBuffer);
                        if (result >= 0 && outputBuffer) {
                            int data_size = av_samples_get_buffer_size(nullptr, audioCodecContext->ch_layout.nb_channels, pFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);

                            char* dt = new char[data_size];
                            memcpy(dt,(char*)outputBuffer,data_size);
                            std::shared_ptr<char> ptr(dt);
                            emit audioDataSend(ptr,data_size);
                            av_free(outputBuffer);
                        }
                        // 转换音频数据为 PCM 并写入 audioIODevice
                        int data_size = av_samples_get_buffer_size(nullptr, audioCodecContext->ch_layout.nb_channels, pFrame->nb_samples, audioCodecContext->sample_fmt, 1);

                        // audioClock = pFrame->best_effort_timestamp * av_q2d(pFormatContext->streams[audioStreamIndex]->time_base);
                    }
                }
            }

            av_packet_unref(pPacket);
        }
        av_frame_free(&pFrame);
        av_packet_free(&pPacket);
        mutex.unlock();
    }
}

int DecodeThread::convertAudioFormat(AVFrame *frame, AVCodecContext *codecContext, uint8_t **outputBuffer) {
    // 创建并初始化 SwrContext
    SwrContext* swrContext = swr_alloc();
    if (!swrContext) {
        qWarning("Failed to allocate SwrContext");
        return -1;
    }

    // 设置 SwrContext 选项
    av_opt_set_chlayout(swrContext, "in_chlayout", &codecContext->ch_layout, 0);
    av_opt_set_sample_fmt(swrContext, "in_sample_fmt", codecContext->sample_fmt, 0);
    av_opt_set_int(swrContext, "in_sample_rate", codecContext->sample_rate, 0);

    av_opt_set_chlayout(swrContext, "out_chlayout", &codecContext->ch_layout, 0);
    av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(swrContext, "out_sample_rate", codecContext->sample_rate, 0);

    // 初始化 SwrContext
    if (swr_init(swrContext) < 0) {
        qWarning("Failed to initialize SwrContext");
        swr_free(&swrContext);
        return -1;
    }

    // 计算输出缓冲区大小并分配
    int outputBufferSize = av_samples_get_buffer_size(
        nullptr,
        codecContext->ch_layout.nb_channels, // 输出通道数
        frame->nb_samples,
        AV_SAMPLE_FMT_S16,
        0
        );

    *outputBuffer = (uint8_t*)av_malloc(outputBufferSize);
    if (!*outputBuffer) {
        qWarning("Failed to allocate output buffer");
        swr_free(&swrContext);
        return -1;
    }

    // 执行格式转换
    int result = swr_convert(
        swrContext,
        outputBuffer,
        frame->nb_samples,
        (const uint8_t**)frame->data,
        frame->nb_samples
        );

    if (result < 0) {
        qWarning("Failed to convert audio format");
        av_free(*outputBuffer);
        *outputBuffer = nullptr;
    }

    // 释放 SwrContext
    swr_free(&swrContext);
    return result;
}
