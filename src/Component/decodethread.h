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
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
#include "Component/playaudiothread.h"
#include "Component/playvideothread.h"
#include "Component/dataqueue.h"
#include "Component/videowidget.h"
class VideoWidget;
class DecodeThread : public QThread {
    Q_OBJECT
signals:
    void frameDecoded(QSharedPointer<AVFrame>);
    void audioDataSend(std::shared_ptr<char> data,int len);
public:
    DecodeThread(QObject* parent = nullptr);
    ~DecodeThread();

    void setFileName(QString file);

    QString getFileName();

    void setFormatContext(AVFormatContext* formatContext);

    void bindVideoWidget(VideoWidget* widget);

    void bindPlayThread(PlayAudioThread *audio,PlayVideoThread *video);

    AVCodecContext* getAudioCodecContext();

    void pause();
    void resume();
    void stop();

public slots:
    void seek(int64_t timestamp);

protected:
    void run() override;

private:
    AVFormatContext* m_avFormatCxt;
    AVCodecContext* videoCodecContext;
    AVCodecContext* audioCodecContext;

    QString m_file;

    DataQueue<AVFrame*> *audioQueue;
    DataQueue<AVFrame*> *videoQueue;

    VideoWidget* videoWidget;
    QMutex mutex;
    int videoStreamIndex;
    int audioStreamIndex;

    PlayAudioThread *audioThread;
    PlayVideoThread *videoThread;

    std::atomic<bool> stopped;
    std::atomic<bool> seekRequested;
    bool playStateChanged = false;
    int64_t seekTime;
    void initDecode();
};




#endif // DECODETHREAD_H
