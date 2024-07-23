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

    void setFormatContext(AVFormatContext* formatContext);

    void bindVideoWidget(VideoWidget* widget);

    void bindPlayThread(PlayAudioThread *audio,PlayVideoThread *video);

    void seek(int64_t timestamp);

    AVCodecContext* getAudioCodecContext();
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
    QWaitCondition condition;
    bool stop;
    int videoStreamIndex;
    int audioStreamIndex;

    void initDecode();
};




#endif // DECODETHREAD_H
