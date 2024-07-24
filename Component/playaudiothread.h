#ifndef PLAYAUDIOTHREAD_H
#define PLAYAUDIOTHREAD_H

#include <QThread>
#include <QAudioSink>
#include <QIODevice>
#include "Component/dataqueue.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#include <QSharedPointer>
#include <Component/audioplayer.h>
#include <QMutex>
#include <QWaitCondition>
class PlayAudioThread : public QThread {
    Q_OBJECT

signals:
    void sigAudioData(const char*,int);
public:
    PlayAudioThread(QObject* parent = nullptr);
    ~PlayAudioThread();

    void setQueues(DataQueue<AVFrame*>* audioQueue);

    void setCodecContext(AVCodecContext* audioCodecContext, AVFormatContext* pFormatContext, int audioStreamIndex);

    int64_t getAudioPts();

    void pause();

    void resume();

    void stop();
public slots:
    bool playAudio();
protected:
    void run() override;

private:
    DataQueue<AVFrame*>* audioQueue;
    bool stopFlag;
    bool quitFlag;
    AVCodecContext* audioCodecContext;
    AVFormatContext* pFormatContext;
    int audioStreamIndex;

    int64_t startTime;
    int64_t stopTime;
    int64_t resumeTime;
    int64_t audioPts;
    // int64_t videoPts;
    bool bindDevice = false;
    void init();
    QMutex mutex;
    QWaitCondition condition;
    AudioPlayer *audioPlayer;
};

// 函数：将音频样本格式转换为 16 位整型
int convertAudioFormat(AVFrame *frame, AVCodecContext *codecContext, uint8_t **outputBuffer);
#endif // PLAYAUDIOTHREAD_H
