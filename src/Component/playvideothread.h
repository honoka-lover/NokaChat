#ifndef PLAYVIDEOTHREAD_H
#define PLAYVIDEOTHREAD_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#include <QThread>
#include "Component/dataqueue.h"
#include "Component/playaudiothread.h"
#include <QWaitCondition>
class PlayVideoThread : public QThread
{
    Q_OBJECT
public:
    PlayVideoThread(QObject* parent = nullptr);
    ~PlayVideoThread();

    void setQueues(DataQueue<AVFrame*>* videoQueue);
    void setCodecContext(AVFormatContext* pFormatContext, int videoStreamIndex);
    void bindAudio(PlayAudioThread*au);
    void stop();
    void resume();
    void pause();
public slots:
    bool playVideo();
signals:
    void frameDecoded(QSharedPointer<AVFrame> frame);

protected:
    void run() override;
    // void timerEvent(QTimerEvent *event) override;

private:



    DataQueue<AVFrame*>* videoQueue;

    bool stopFlag;
    AVCodecContext* audioCodecContext;
    AVFormatContext* pFormatContext;

    int videoStreamIndex;

    int64_t startTime;
    int64_t stopTime;
    int64_t resumeTime;
    int64_t audioPts;
    int64_t videoPts;
    QMutex mutex;
    PlayAudioThread *audio;
    QWaitCondition condition;
};

#endif // PLAYVIDEOTHREAD_H
