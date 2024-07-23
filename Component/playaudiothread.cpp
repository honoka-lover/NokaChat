#include "Component/playaudiothread.h"

#include <QDebug>

PlayAudioThread::PlayAudioThread(QObject* parent)
    : QThread(parent), audioSink(nullptr), audioIODevice(nullptr), stop(false),
    audioCodecContext(nullptr), pFormatContext(nullptr), audioStreamIndex(-1) {

    startTime = av_gettime();
    audioPts = AV_NOPTS_VALUE;
}

PlayAudioThread::~PlayAudioThread() {
    stop = true;
    if (audioSink) {
        audioSink->stop();
    }
}

void PlayAudioThread::setQueues(DataQueue<AVFrame*>* audioQueue) {
    this->audioQueue = audioQueue;
}

void PlayAudioThread::setAudioSink(QAudioSink* audioSink, QIODevice* audioIODevice) {
    this->audioSink = audioSink;
    this->audioIODevice = audioIODevice;
}

void PlayAudioThread::setCodecContext(AVCodecContext* audioCodecContext, AVFormatContext* pFormatContext, int audioStreamIndex) {
    this->audioCodecContext = audioCodecContext;
    this->pFormatContext = pFormatContext;
    this->audioStreamIndex = audioStreamIndex;
}

int64_t PlayAudioThread::getAudioPts()
{
    return this->audioPts;
}

void PlayAudioThread::run() {
    if(audioStreamIndex < 0){
        qDebug()<<"没有音频数据流，音频播放线程退出";
        return;
    }
    while(true){
        if(!playAudio())
            msleep(10);
    }
}


bool PlayAudioThread::playAudio() {
    if (!audioQueue->empty()) {
        AVFrame* audioFrame = audioQueue->pop();
        // // 转换音频数据为 PCM 并写入 audioIODevice
        uint8_t* outputBuffer = nullptr;
        int result = convertAudioFormat(audioFrame, audioCodecContext, &outputBuffer);
        if (result >= 0 && outputBuffer) {
            int data_size = av_samples_get_buffer_size(nullptr, audioCodecContext->ch_layout.nb_channels, audioFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);

            // 获取帧的 PTS
            if (audioPts == AV_NOPTS_VALUE) {
                audioPts = audioFrame->pts;
            }

            int64_t delay = (audioFrame->pts - audioPts) * av_q2d(pFormatContext->streams[audioStreamIndex]->time_base) * 1000000;
            int64_t currentTime = av_gettime() - startTime;
            int64_t sleepTime = delay-currentTime;
            if (sleepTime>0) {
                QThread::usleep(sleepTime);
            }


            audioIODevice->write((char*)outputBuffer, data_size);
            av_free(outputBuffer);
        }
        av_frame_free(&audioFrame);
        return true;
    }else
        return false;
}

int convertAudioFormat(AVFrame *frame, AVCodecContext *codecContext, uint8_t **outputBuffer) {
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
