#include "decodethread.h"
#include <QDebug>
#include <QTimer>
#include <QMediaDevices>
AVFormatContext* open_video_file(const char* filename) {
    AVFormatContext* pFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pFormatContext, filename, nullptr, nullptr) != 0) {
        return nullptr;
    }
    return pFormatContext;
}

DecodeThread::DecodeThread(QObject* parent)
    : QThread(parent),
    m_avFormatCxt(nullptr),
    audioCodecContext(nullptr),
    videoWidget(nullptr),
    stopFlag(false),
    videoStreamIndex(-1),
    audioStreamIndex(-1),
    videoThread(nullptr),
    audioThread(nullptr),
    m_file(""),
    quitFlag(false)
{
    size_t maxQueueSize = 100; // Set the maximum queue size
    audioQueue = new DataQueue<AVFrame*>(100);
    videoQueue = new DataQueue<AVFrame*>(100);

}

DecodeThread::~DecodeThread() {
    mutex.lock();
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
    }
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
    }
    if (m_avFormatCxt) {
        avformat_close_input(&m_avFormatCxt);
        avformat_free_context(m_avFormatCxt);
    }
    stopFlag = false;
    quitFlag = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
    audioQueue->stop();
    while(!audioQueue->empty()){
        AVFrame *frame;
        bool ok = audioQueue->pop(frame);
        if(ok)
            av_frame_free(&frame);
    }
    videoQueue->stop();
    while(!videoQueue->empty()){
        AVFrame *frame;
        bool ok = videoQueue->pop(frame);
        if(ok)
            av_frame_free(&frame);
    }


}

void DecodeThread::setFileName(QString file)
{
    if(!m_file.isEmpty()){
        avformat_close_input(&m_avFormatCxt);
    }
    m_file = file;
    m_avFormatCxt = open_video_file(m_file.toStdString().c_str());
    initDecode();
}

void DecodeThread::initDecode() {
    videoStreamIndex = av_find_best_stream(m_avFormatCxt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStreamIndex = av_find_best_stream(m_avFormatCxt, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    if (videoStreamIndex >= 0) {
        AVCodecParameters* codecParameters = m_avFormatCxt->streams[videoStreamIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
        videoCodecContext = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(videoCodecContext, codecParameters);
        avcodec_open2(videoCodecContext, codec, nullptr);
    }

    if (audioStreamIndex >= 0) {
        AVCodecParameters* audioCodecParameters = m_avFormatCxt->streams[audioStreamIndex]->codecpar;
        const AVCodec* audioCodec = avcodec_find_decoder(audioCodecParameters->codec_id);
        audioCodecContext = avcodec_alloc_context3(audioCodec);
        avcodec_parameters_to_context(audioCodecContext, audioCodecParameters);
        avcodec_open2(audioCodecContext, audioCodec, nullptr);

    }

}

void DecodeThread::bindVideoWidget(VideoWidget* widget) {
    if(widget){
        videoWidget = widget;
    }   
}

void DecodeThread::bindPlayThread(PlayAudioThread *audio,PlayVideoThread *video)
{
    if(audio){
        audio->setQueues(audioQueue);
        audio->setCodecContext(audioCodecContext, m_avFormatCxt, audioStreamIndex);
        audioThread = audio;
    }
    if(video){
        video->setQueues(videoQueue);
        video->setCodecContext(m_avFormatCxt, videoStreamIndex);
        video->bindAudio(audio);
        videoThread = video;
    }

}

void DecodeThread::seek(int64_t timestamp) {
    QMutexLocker locker(&mutex);
    if (m_avFormatCxt) {
        av_seek_frame(m_avFormatCxt, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(videoCodecContext);
        avcodec_flush_buffers(audioCodecContext);
        // condition.wakeOne();
    }
}

AVCodecContext *DecodeThread::getAudioCodecContext()
{
    return this->audioCodecContext;
}

void DecodeThread::pause()
{
    mutex.lock();
    stopFlag = true;
    mutex.unlock();
}

void DecodeThread::run() {
    while (!isInterruptionRequested()) {
        {
            QMutexLocker locker(&mutex);
            if (stopFlag) {
                condition.wait(&mutex);
            }
            if(quitFlag){
                return;
            }
        }
        AVPacket* pPacket = av_packet_alloc();
        AVFrame* pFrame = av_frame_alloc();
        if (av_read_frame(m_avFormatCxt, pPacket) >= 0) {
            if (pPacket->stream_index == videoStreamIndex) {
                if (avcodec_send_packet(videoCodecContext, pPacket) >= 0) {
                    while (avcodec_receive_frame(videoCodecContext, pFrame) >= 0) {
                        AVFrame* frame = av_frame_clone(pFrame);
                        videoQueue->push(frame);
                    }
                }
            }else if (pPacket->stream_index == audioStreamIndex) {
                if (avcodec_send_packet(audioCodecContext, pPacket) >= 0) {
                    while (avcodec_receive_frame(audioCodecContext, pFrame) >= 0) {
                        AVFrame* frame = av_frame_clone(pFrame);
                        audioQueue->push(frame);
                    }
                }
            }

            av_packet_unref(pPacket);
        }
        av_frame_free(&pFrame);
        av_packet_free(&pPacket);
    }
}

QString DecodeThread::getFileName() {
    return this->m_file;
}

void DecodeThread::stop() {
    QMutexLocker locker(&mutex);
    stopFlag = false;
    quitFlag = true;
    audioThread->stop();
    videoThread->stop();
    audioQueue->stop();
    videoQueue->stop();
    condition.wakeAll();
}

void DecodeThread::resume() {
    QMutexLocker locker(&mutex);
    stopFlag = false;
    condition.wakeAll();
}
