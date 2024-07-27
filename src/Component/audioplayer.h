#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QThread>
#include <QAudioSink>
#include <QIODevice>
#include <QMutex>
#include "Component/dataqueue.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#include <QSharedPointer>

class AudioPlayer:public QObject
{
    Q_OBJECT
public:
    AudioPlayer(QObject* parent = nullptr);
    ~AudioPlayer();
    void init();

    void setAudioSink(QAudioSink* audioSink, QIODevice* audioIODevice);

    void resume();
    void stop();

    void clearData();
    void setVolumn(int);
public slots:
    void audioWrite(const char* data,int len);
private:
    QAudioSink* audioSink;
    QIODevice* audioIODevice;
    QMutex m_mutex;
};

#endif // AUDIOPLAYER_H
