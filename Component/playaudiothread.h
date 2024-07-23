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
#include <QTimer>

class PlayAudioThread : public QThread {
    Q_OBJECT
public:
    PlayAudioThread(QObject* parent = nullptr);
    ~PlayAudioThread();

    void setQueues(DataQueue<AVFrame*>* audioQueue);
    void setAudioSink(QAudioSink* audioSink, QIODevice* audioIODevice);
    void setCodecContext(AVCodecContext* audioCodecContext, AVFormatContext* pFormatContext, int audioStreamIndex);

    int64_t getAudioPts();

protected:
    void run() override;

private:
    bool playAudio();

    DataQueue<AVFrame*>* audioQueue;
    QAudioSink* audioSink;
    QIODevice* audioIODevice;
    bool stop;

    AVCodecContext* audioCodecContext;
    AVFormatContext* pFormatContext;
    int audioStreamIndex;

    int64_t startTime;
    int64_t audioPts;
    // int64_t videoPts;
};

// 函数：将音频样本格式转换为 16 位整型
int convertAudioFormat(AVFrame *frame, AVCodecContext *codecContext, uint8_t **outputBuffer);
#endif // PLAYAUDIOTHREAD_H
