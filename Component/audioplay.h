#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H

#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
#include "Component/decodethread.h"
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class AudioPlay:public QObject
{
    Q_OBJECT
public:
    explicit AudioPlay(DecodeThread *decode);

    ~AudioPlay();

    void init();
private slots:
    void playData(std::shared_ptr<char> data,int len);


private:

    QAudioOutput* audioOutput;
    QIODevice* audioIODevice;
    QAudioSink * audioSink;
    DecodeThread *decode;



};

#endif // AUDIOPLAY_H
