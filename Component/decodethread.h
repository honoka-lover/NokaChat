#ifndef DECODETHREAD_H
#define DECODETHREAD_H

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
class VideoWidget;
class DecodeThread : public QThread {
    Q_OBJECT
signals:
    void frameDecoded(QSharedPointer<AVFrame>);
    void audioDataSend(std::shared_ptr<char> data,int len);
public:
    DecodeThread(QObject* parent = nullptr);
    ~DecodeThread();

    void setFormatContext(AVFormatContext* formatContext);
    void setVideoWidget(VideoWidget* widget);

    void seek(int64_t timestamp);

    AVCodecContext* getAudioCodecContext();
protected:
    void run() override;

private:
    AVFormatContext* pFormatContext;
    AVCodecContext* videoCodecContext;
    AVCodecContext* audioCodecContext;

    VideoWidget* videoWidget;
    QMutex mutex;
    QWaitCondition condition;
    bool stop;
    int videoStreamIndex;
    int audioStreamIndex;
    double audioClock;  // 用于跟踪音频时钟


    // 函数：将音频样本格式转换为 16 位整型
    int convertAudioFormat(AVFrame* frame, AVCodecContext* codecContext, uint8_t** outputBuffer);
};




#endif // DECODETHREAD_H
