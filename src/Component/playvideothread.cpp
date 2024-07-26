#include "playvideothread.h"
#include <QTimer>
#include <QSharedPointer>
#include <QDebug>
PlayVideoThread::PlayVideoThread(QObject* parent)
    : QThread(parent),
    stopFlag(false),
    audio(nullptr),
    audioCodecContext(nullptr),
    pFormatContext(nullptr),
    videoStreamIndex(-1) {

    startTime = av_gettime();
    stopTime = av_gettime();
    resumeTime = av_gettime();
    videoPts = AV_NOPTS_VALUE;
    audioPts = AV_NOPTS_VALUE;
}

PlayVideoThread::~PlayVideoThread() {
    stop();
}

void PlayVideoThread::setQueues(DataQueue<AVFrame*>* videoQueue) {
    this->videoQueue = videoQueue;
}


void PlayVideoThread::setCodecContext(AVFormatContext* pFormatContext, int videoStreamIndex) {
    this->pFormatContext = pFormatContext;
    this->videoStreamIndex = videoStreamIndex;
}

void PlayVideoThread::bindAudio(PlayAudioThread *au)
{
    audio = au;
}

void PlayVideoThread::pause()
{
    mutex.lock();
    stopFlag = true;
    stopTime = av_gettime();
    mutex.unlock();
}

void PlayVideoThread::run() {
    while (!isInterruptionRequested()) {
        {
            QMutexLocker locker(&mutex);
            if (stopFlag) {
                condition.wait(&mutex);
            }
        }
        if(videoStreamIndex == -1){
            QThread::usleep(10000);
        }else if(videoStreamIndex<0){
            qDebug()<<"没有视频数据流，视频播放线程退出";
            return;
        }
        if(!playVideo())
            QThread::usleep(10000);
    }
}

bool PlayVideoThread::playVideo() {
    if (!videoQueue->empty()) {
        AVFrame *videoFrame;
        bool ok = videoQueue->pop(videoFrame);
        if (ok) {
            QSharedPointer<AVFrame> frameCopy(av_frame_clone(videoFrame), [](AVFrame *frame) {
                av_frame_free(&frame);
            });
            if (videoPts == AV_NOPTS_VALUE) {
                videoPts = videoFrame->pts;
            }


            int64_t delay =
                    (videoFrame->pts - videoPts) * av_q2d(pFormatContext->streams[videoStreamIndex]->time_base) *
                    1000000;
            int64_t currentTime = av_gettime() - startTime;

            if (delay > currentTime) {
                QThread::usleep(delay - currentTime);
            }


            emit frameDecoded(frameCopy);
            av_frame_free(&videoFrame);
            return true;
        }
    }
    return false;
}

void PlayVideoThread::resume() {
    QMutexLocker locker(&mutex);
    stopFlag = false;
    resumeTime = av_gettime();
    startTime = startTime + resumeTime - stopTime;
    condition.wakeAll();
}

void PlayVideoThread::stop() {
    videoQueue->stop();
    requestInterruption();
    resume();
}
