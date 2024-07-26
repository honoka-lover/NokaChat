#include "audioplayer.h"
#include <QDebug>
AudioPlayer::AudioPlayer(QObject* parent):
    QObject(parent),
    audioSink(nullptr),
    audioIODevice(nullptr)
{}

AudioPlayer::~AudioPlayer()
{
    if(audioSink){
        audioSink->stop();
        delete audioSink;
    }
}

void AudioPlayer::setAudioSink(QAudioSink *audioSink, QIODevice *audioIODevice)
{
    this->audioSink = audioSink;
    this->audioIODevice = audioIODevice;
}

void AudioPlayer::stop()
{
    if(audioSink)
        audioSink->stop();
}

void AudioPlayer::audioWrite(const char *data, int len)
{
    if(audioIODevice)
        audioIODevice->write(data,len);
    else
        qDebug()<<"播放设备为初始化";
}

void AudioPlayer::resume() {
    if(audioSink)
        audioIODevice = audioSink->start();
}
