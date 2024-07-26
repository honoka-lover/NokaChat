#include "videotool.h"
#include "ui_videotool.h"
#include <QTime>
VideoTool::VideoTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoTool)
{
    ui->setupUi(this);

    ui->videoToolBack->setObjectName("videoToolBack");
    ui->pauseButton->setObjectName("musicPlay");
    ui->leftButton->setObjectName("lastButton");
    ui->rightButton->setObjectName("nextButton");
    ui->volumnButton->setObjectName("trumpt");
}

VideoTool::~VideoTool()
{
    delete ui;
}

void VideoTool::setAllTime(int64_t duration)
{
    allTime = duration;
    startTime = av_gettime();
    // 转换为小时、分钟和秒
    long long hours = duration / AV_TIME_BASE /3600;
    long long minutes = (duration /AV_TIME_BASE ) % 3600 / 60;
    long long seconds = duration /AV_TIME_BASE % 60;
    durationStr = QString("%1:%2:%3").arg(hours,2,10, QChar('0')).arg(minutes,2,10, QChar('0')).arg(seconds,2,10, QChar('0'));
    ui->timeLabel->setText("00:00:00 / "+ durationStr);
}

void VideoTool::on_pauseButton_clicked()
{

}


void VideoTool::on_rightButton_clicked()
{

}


void VideoTool::on_leftButton_clicked()
{

}


void VideoTool::on_timeSlider_sliderReleased()
{

}

