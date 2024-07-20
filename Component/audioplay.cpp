#include "audioplay.h"
#include <QMediaDevices>
#include <QDebug>
AudioPlay::AudioPlay(DecodeThread *decode):
    audioSink(nullptr),
    audioIODevice(nullptr),
    decode(decode)
{
    connect(decode,&DecodeThread::audioDataSend,this,&AudioPlay::playData);
}

AudioPlay::~AudioPlay()
{
    if (audioSink) {
        audioSink->stop();
        delete audioSink;
    }
}

void AudioPlay::playData(std::shared_ptr<char> data, int len)
{
    audioIODevice->write(data.get(), len);
}

void AudioPlay::init()
{
    if(decode == nullptr)
    {
        qDebug()<<"音频未获取到解码线程指针";
        return;
    }
    AVCodecContext* audioCodecContext = decode->getAudioCodecContext();
    // 初始化音频输出设备
    QAudioFormat format;
    format.setSampleRate(audioCodecContext->sample_rate);
    format.setChannelCount(audioCodecContext->ch_layout.nb_channels);
    // 设置样本格式
    format.setSampleFormat(QAudioFormat::Int16);
    // format.setChannelConfig(audioCodecContext->ch_layout.order)

    audioSink = new QAudioSink(format,this);
    audioIODevice = audioSink->start();
}
