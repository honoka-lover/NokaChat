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
    setVolumn(100);
}

void AudioPlayer::stop()
{
    if(audioSink)
        audioSink->stop();
}

void AudioPlayer::clearData()
{
    if(audioIODevice){
        m_mutex.lock();

        audioSink->stop();
        audioIODevice->close();
        audioIODevice = audioSink->start();
        audioIODevice->open(QIODevice::ReadWrite);
        m_mutex.unlock();
    }
}

void AudioPlayer::setVolumn(int value)
{
    if(audioSink)
        audioSink->setVolume(value / 100.0f);
}

void AudioPlayer::audioWrite(const char *data, int len)
{
    m_mutex.lock();
    if(audioIODevice)
        audioIODevice->write(data,len);
    else
        qDebug()<<"播放设备为初始化";
    m_mutex.unlock();
}

void AudioPlayer::resume() {
    if(audioSink)
        audioIODevice = audioSink->start();
}
