#include "decodethread.h"
#include <QDebug>
#include <QTime>
DecodeThread::DecodeThread(QObject* parent)
    : QThread(parent), pFormatContext(nullptr), videoWidget(nullptr), stop(false) {
}

DecodeThread::~DecodeThread() {
    mutex.lock();
    stop = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

void DecodeThread::setFormatContext(AVFormatContext* formatContext) {
    QMutexLocker locker(&mutex);
    pFormatContext = formatContext;
    condition.wakeOne();
}

void DecodeThread::setVideoWidget(VideoWidget* widget) {
    QMutexLocker locker(&mutex);
    videoWidget = widget;
    condition.wakeOne();
}

void DecodeThread::run() {
    AVCodecContext* pCodecContext = nullptr;
    int videoStreamIndex = -1;
    AVPacket* pPacket = av_packet_alloc();
    AVFrame* pFrame = av_frame_alloc();
    // QTime frameTimer;

    for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
        AVStream* stream = pFormatContext->streams[i];
        const AVCodec* pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (pCodec && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            pCodecContext = avcodec_alloc_context3(pCodec);
            if (avcodec_parameters_to_context(pCodecContext, stream->codecpar) < 0) {
                return;
            }
            if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
                return;
            }
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        return;
    }

    while (true) {
        mutex.lock();
        if (stop) {
            mutex.unlock();
            break;
        }
        mutex.unlock();

        if (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == videoStreamIndex) {
                if (avcodec_send_packet(pCodecContext, pPacket) >= 0) {
                    while (avcodec_receive_frame(pCodecContext, pFrame) >= 0) {
                        int64_t pts = pFrame->best_effort_timestamp;
                        AVRational timeBase = pFormatContext->streams[videoStreamIndex]->time_base;
                        int64_t delay = (pts * av_q2d(timeBase)); // 延迟时间，单位为毫秒

                        // 等待以确保视频帧按时间戳显示
                        QThread::msleep(delay);

                        QSharedPointer<AVFrame> frameCopy(av_frame_clone(pFrame), [](AVFrame* frame){
                            av_frame_free(&frame);
                        });
                        if (!frameCopy.isNull()) {
                            emit frameDecoded(frameCopy);
                        }
                    }
                }
            }
            av_packet_unref(pPacket);
        } else {
            break;
        }
    }

    av_frame_free(&pFrame);
    av_packet_free(&pPacket);
    avcodec_free_context(&pCodecContext);
}
