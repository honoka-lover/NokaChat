#include "Component/playaudiothread.h"

#include <QDebug>
#include <QTimer>
#include <QMediaDevices>
PlayAudioThread::PlayAudioThread(QObject* parent):
    QThread(parent),
    stopped(false),
    audioCodecContext(nullptr),
    pFormatContext(nullptr),
    audioStreamIndex(-1)
{
    audioPlayer = new AudioPlayer;
    connect(this,&PlayAudioThread::sigAudioData,audioPlayer,&AudioPlayer::audioWrite);
    startTime = av_gettime();
    stopTime = av_gettime();
    resumeTime = av_gettime();
    audioPts = AV_NOPTS_VALUE;
}

PlayAudioThread::~PlayAudioThread() {
    stop();
}

void PlayAudioThread::setQueues(DataQueue<AVFrame*>* audioQueue) {
    this->audioQueue = audioQueue;
}

void PlayAudioThread::setCodecContext(AVCodecContext* audioCodecContext, AVFormatContext* pFormatContext, int audioStreamIndex) {
    this->audioCodecContext = audioCodecContext;
    this->pFormatContext = pFormatContext;
    this->audioStreamIndex = audioStreamIndex;

    init();
}

int64_t PlayAudioThread::getAudioPts()
{
    return this->audioPts;
}

void PlayAudioThread::pause()
{
    mutex.lock();
    stopped = true;
    stopTime = av_gettime();
    mutex.unlock();
//    audioPlayer->stop();
}

void PlayAudioThread::run() {
    while (!isInterruptionRequested()) {
        if (stopped) {
            QThread::msleep(10);
            continue;
        }
        if(audioStreamIndex == -1){
            QThread::usleep(10000);
        }else if(audioStreamIndex < 0){
            qDebug()<<"没有音频数据流，音频播放线程退出";
            return;
        }
        if(!playAudio())
            QThread::usleep(10000);
    }
}

void PlayAudioThread::init()
{
    if(!audioCodecContext){
        return;
    }else if(!bindDevice){
        // 初始化音频输出设备
        QAudioFormat format;
        format.setSampleRate(audioCodecContext->sample_rate);
        format.setChannelCount(audioCodecContext->ch_layout.nb_channels);
        // 设置样本格式
        format.setSampleFormat(QAudioFormat::Int16);
        // format.setChannelConfig(audioCodecContext->ch_layout.order)

        QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
        auto audioSink = new QAudioSink(audioDevice, format);

        auto audioIODevice = audioSink->start();

        audioPlayer->setAudioSink(audioSink,audioIODevice);
        bindDevice = true;
    }
}


bool PlayAudioThread::playAudio() {
    if (!audioQueue->empty()) {
        AVFrame *audioFrame;
        bool ok = audioQueue->pop(audioFrame);
        if (ok) {
            // 转换音频数据为 PCM 并写入 audioIODevice
            uint8_t *outputBuffer = nullptr;
            int result = convertAudioFormat(audioFrame, audioCodecContext, &outputBuffer);
            if (result >= 0 && outputBuffer) {
                int data_size = av_samples_get_buffer_size(nullptr, audioCodecContext->ch_layout.nb_channels,
                                                           audioFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);

                // 获取帧的 PTS
                if (audioPts == AV_NOPTS_VALUE) {
                    audioPts = audioFrame->pts;
                }

                int64_t delay =
                        (audioFrame->pts - audioPts) * av_q2d(pFormatContext->streams[audioStreamIndex]->time_base) *
                        1000000;
                int64_t currentTime = av_gettime() - startTime;
                int64_t sleepTime = delay - currentTime;
                if (sleepTime > 0) {
                    QThread::usleep(sleepTime);
                }

                emit sigAudioData((char *) outputBuffer, data_size);
                av_free(outputBuffer);
            }
            av_frame_free(&audioFrame);
            return true;
        }
    }
    return false;

}

void PlayAudioThread::setVolumn(int value)
{
    if(audioPlayer)
        audioPlayer->setVolumn(value);
}

void PlayAudioThread::resume() {
    QMutexLocker locker(&mutex);
    stopped = false;
    resumeTime = av_gettime();
    startTime = startTime + resumeTime -stopTime;
//    audioPlayer->resume();
    condition.wakeAll();
}

void PlayAudioThread::stop() {
    requestInterruption();
    resume();
    audioQueue->stop();
    if(audioPlayer)
        audioPlayer->stop();
    delete audioPlayer;
    audioPlayer = nullptr;
}

void PlayAudioThread::updateTimeStamp(int64_t stamp)
{
    mutex.lock();
    startTime = av_gettime() - stamp;
    audioPlayer->clearData();
    mutex.unlock();
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
