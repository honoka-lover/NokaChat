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
    videoStreamIndex(-1),
    audioStreamIndex(-1),
    videoThread(nullptr),
    audioThread(nullptr),
    m_file(""),
    seekTime(0),
    stopped(false),
    seekRequested(false)
{
    size_t maxQueueSize = 100; // Set the maximum queue size
    audioQueue = new DataQueue<AVFrame*>(100);
    videoQueue = new DataQueue<AVFrame*>(100);
    //默认关闭阻塞等待
    audioQueue->stop();
    videoQueue->stop();
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
    // 如果duration为负数，手动计算总时长
    if (m_avFormatCxt->duration < 0) {
        int64_t maxDuration = 0;
        for (unsigned int i = 0; i < m_avFormatCxt->nb_streams; ++i) {
            AVStream *stream = m_avFormatCxt->streams[i];
            if (stream->duration != AV_NOPTS_VALUE) {
                int64_t duration = av_rescale_q(stream->duration, stream->time_base, AV_TIME_BASE_Q);
                maxDuration = std::max(maxDuration, duration);
            }
        }
        m_avFormatCxt->duration = maxDuration;
    }
    if(videoWidget){
        videoWidget->setAllTime(m_avFormatCxt->duration);
    }
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
    playStateChanged = stopped;
    stopped = true;
    seekTime = timestamp;
    videoThread->pause();
    audioThread->pause();
    if(videoThread)
        videoThread->updateTimeStamp(timestamp);
    if(audioThread)
        audioThread->updateTimeStamp(timestamp);
    while(!audioQueue->empty()){
        AVFrame *frame;
        bool ok = audioQueue->pop(frame);
        if(ok)
            av_frame_free(&frame);
    }
    while(!videoQueue->empty()){
        AVFrame *frame;
        bool ok = videoQueue->pop(frame);
        if(ok)
            av_frame_free(&frame);
    }
    seekRequested = true;
}

AVCodecContext *DecodeThread::getAudioCodecContext()
{
    return this->audioCodecContext;
}

void DecodeThread::pause()
{
    stopped = true;
}

void DecodeThread::run() {
    while (!isInterruptionRequested()) {
        if (seekRequested) {
            QMutexLocker locker(&mutex);
            av_seek_frame(m_avFormatCxt, -1, seekTime, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(audioCodecContext);
            avcodec_flush_buffers(videoCodecContext);
            seekRequested = false;
            stopped = playStateChanged;
            if(!stopped){
                videoThread->resume();
                audioThread->resume();
            }
        }

        if (stopped) {
            QThread::msleep(10);
            continue;
        }

        AVPacket* packet = av_packet_alloc();
        if (av_read_frame(m_avFormatCxt, packet) >= 0) {
            if (packet->stream_index == audioStreamIndex) {
                AVFrame* frame = av_frame_alloc();
                int ret = avcodec_send_packet(audioCodecContext, packet);
                if (ret >= 0) {
                    ret = avcodec_receive_frame(audioCodecContext, frame);
                    if (ret >= 0) {
                        while(!audioQueue->push(frame)){
                            msleep(10);
                            if(stopped)
                                break;
                        }
                    } else {
                        av_frame_free(&frame);
                    }
                } else {
                    av_frame_free(&frame);
                }
            } else if (packet->stream_index == videoStreamIndex) {
                AVFrame* frame = av_frame_alloc();
                int ret = avcodec_send_packet(videoCodecContext, packet);
                if (ret >= 0) {
                    ret = avcodec_receive_frame(videoCodecContext, frame);
                    if (ret >= 0) {
                        while(!videoQueue->push(frame)){
                            msleep(10);
                            if(stopped)
                                break;
                        }
                    } else {
                        av_frame_free(&frame);
                    }
                } else {
                    av_frame_free(&frame);
                }
            }
            av_packet_unref(packet);
        }
        av_packet_free(&packet);
//            if (pPacket->stream_index == videoStreamIndex) {
//                if (avcodec_send_packet(videoCodecContext, pPacket) >= 0) {
//                    while (avcodec_receive_frame(videoCodecContext, pFrame) >= 0) {
//                        AVFrame* frame = av_frame_clone(pFrame);
//                        videoQueue->push(frame);
//                    }
//                }
//            }else if (pPacket->stream_index == audioStreamIndex) {
//                if (avcodec_send_packet(audioCodecContext, pPacket) >= 0) {
//                    while (avcodec_receive_frame(audioCodecContext, pFrame) >= 0) {
//                        AVFrame* frame = av_frame_clone(pFrame);
//                        audioQueue->push(frame);
//                    }
//                }
//            }
//
//            av_packet_unref(pPacket);
//        }

    }
}

QString DecodeThread::getFileName() {
    return this->m_file;
}

void DecodeThread::stop() {
    stopped = true;
    audioQueue->stop();
    videoQueue->stop();
    requestInterruption();    
}

void DecodeThread::resume() {
    stopped = false;
}
