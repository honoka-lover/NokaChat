#ifndef DECODETHREAD_H
#define DECODETHREAD_H

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class VideoWidget;
class DecodeThread : public QThread {
    Q_OBJECT
signals:
    void frameDecoded(QSharedPointer<AVFrame>);

public:
    DecodeThread(QObject* parent = nullptr);
    ~DecodeThread();

    void setFormatContext(AVFormatContext* formatContext);
    void setVideoWidget(VideoWidget* widget);

protected:
    void run() override;

private:
    AVFormatContext* pFormatContext;
    VideoWidget* videoWidget;
    QMutex mutex;
    QWaitCondition condition;
    bool stop;
};




#endif // DECODETHREAD_H
