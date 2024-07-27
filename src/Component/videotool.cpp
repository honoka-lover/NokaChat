#include "videotool.h"
#include "ui_videotool.h"
#include <QTime>
#include <QDebug>
#include <QTimer>
VideoTool::VideoTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoTool)
    , startTime(av_gettime())
    , stopTime(startTime)
    , resumeTime(startTime)
    , isPlay(false)
    , allTime(0)
{
    ui->setupUi(this);

    // ui->videoToolBack->setObjectName("videoToolBack");
    ui->pauseButton->setObjectName("musicPlay");
    ui->leftButton->setObjectName("lastButton");
    ui->rightButton->setObjectName("nextButton");
    ui->volumnButton->setObjectName("trumpt");
    ui->timeSlider->setObjectName("musicSlider");
    ui->whiteButton->setObjectName("blankButton");
    ui->whiteButton_2->setObjectName("blankButton");
    ui->pauseButton->setCheckable(true);

    audioSlider = new QSlider(parent);
    // 跟踪滑块
    audioSlider->setTracking(true);
    audioSlider->setRange(0, 100);
    audioSlider->setValue(20);
    audioSlider->setObjectName("myAudio");
    audioSlider->hide();
    audioSlider->setFixedHeight(100);
    connect(audioSlider,&QSlider::sliderReleased,this,&VideoTool::setVolumn);

    m_timer = new QTimer;
    m_timer->setInterval(20);
    connect(m_timer,&QTimer::timeout,this,&VideoTool::upDateSlider);
    ui->timeSlider->setRange(0,1000);
    ui->timeSlider->setValue(0);
    m_timer->start();

    this->installEventFilter(this);
    audioSlider->installEventFilter(this);
    ui->volumnButton->installEventFilter(this);
    ui->videoToolBack->installEventFilter(this);
    ui->timeSlider->installEventFilter(this);
    ui->leftButton->installEventFilter(this);
    ui->rightButton->installEventFilter(this);
    ui->pauseButton->installEventFilter(this);
    ui->timeLabel->installEventFilter(this);
    ui->whiteButton->installEventFilter(this);
    ui->whiteButton_2->installEventFilter(this);
}

VideoTool::~VideoTool()
{
    delete ui;
}

void VideoTool::setAllTime(int64_t duration)
{
    allTime = duration;
    if(allTime == 0)
        return;
    init();
}

void VideoTool::setPauseButtonToggle()
{
    if(allTime == 0)
        return;
    ui->pauseButton->toggle();
}

bool VideoTool::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->volumnButton && event->type() == QEvent::HoverEnter){
        audioSlider->show();
    }else if(watched != audioSlider && watched != ui->volumnButton && event->type() == QEvent::HoverEnter){
        audioSlider->hide();
    }
    return false;
}

void VideoTool::hideEvent(QHideEvent *event)
{
    audioSlider->hide();
}

void VideoTool::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(10,[=](){
        audioSlider->move(ui->volumnButton->pos().x() + 7,((QWidget*)parent())->height() - this->height() - audioSlider->height() + ui->timeSlider->height());
    });

}

void VideoTool::on_rightButton_clicked()
{
    if(allTime == 0)
        return;
    mutex.lock();
    //检查是否暂停
    if(stopTime > resumeTime){
        int64_t delay = stopTime - startTime;
        int64_t addedTime = timeInterval * AV_TIME_BASE;
        if(delay + addedTime > allTime){
            startTime = stopTime - allTime;
        }else{
            startTime = startTime - addedTime;
        }
        emit updateTimeStamp(stopTime - startTime);
        //更新时间进度条
        isPlay = true;
        upDateSlider();
        isPlay = false;
    }else {
        int64_t delay = av_gettime() - startTime;
        int64_t addedTime = timeInterval * AV_TIME_BASE;
        if (delay + addedTime > allTime) {
            startTime = av_gettime() - allTime;
        } else {
            startTime = startTime - addedTime;
        }
        emit updateTimeStamp(av_gettime() - startTime);
    }
    mutex.unlock();
}


void VideoTool::on_leftButton_clicked()
{
    if(allTime == 0)
        return;
    mutex.lock();
    //检查是否暂停
    if(stopTime > resumeTime){
        int64_t delay = stopTime - startTime;
        int64_t reducedTime = timeInterval * AV_TIME_BASE;
        if(reducedTime > delay){
            startTime = stopTime;
            ui->timeLabel->setText("00:00:00 / " +durationStr);
        }else{
            startTime = startTime + reducedTime;
            //更新时间进度条
            isPlay = true;
            upDateSlider();
            isPlay = false;
        }
        emit updateTimeStamp(stopTime - startTime);
    }else{
        int64_t delay = av_gettime() - startTime;
        int64_t reducedTime = timeInterval * AV_TIME_BASE;
        if(reducedTime > delay){
            startTime = av_gettime();
        }else{
            startTime = startTime + reducedTime;
        }
        emit updateTimeStamp(av_gettime() - startTime);
    }
    mutex.unlock();
}


void VideoTool::on_timeSlider_sliderReleased()
{
    mutex.lock();
    isChanging = false;
    int value = ui->timeSlider->value();
    int64_t newTime = value / 1000.0f *allTime;
    emit updateTimeStamp(newTime);
    startTime = av_gettime() - newTime;
    mutex.unlock();
}


void VideoTool::on_pauseButton_toggled(bool checked)
{
    if(checked){
        qDebug()<<"pause";
        ui->pauseButton->setObjectName("musicPause");
        stopTime = av_gettime();
    }else{
        qDebug()<<"play";
        ui->pauseButton->setObjectName("musicPlay");
        resumeTime = av_gettime();
        startTime = startTime + resumeTime - stopTime;
    }
    ui->pauseButton->setStyleSheet(ui->pauseButton->styleSheet());
    emit sigPuase(checked);
    isPlay = !checked;
}

void VideoTool::setVolumn()
{
    auto value = audioSlider->value();
    emit sigVolumn(value);
}

void VideoTool::upDateSlider()
{
    if(isPlay && !isChanging){
        int64_t currentTime = av_gettime();
        int64_t delay = currentTime-startTime;
        if(delay >= allTime){
            delay = allTime;
            startTime = av_gettime() - allTime;
        }
        ui->timeSlider->setValue(delay*1000/allTime);

        long long hours = delay / AV_TIME_BASE /3600;
        long long minutes = (delay /AV_TIME_BASE ) % 3600 / 60;
        long long seconds = delay /AV_TIME_BASE % 60;
        QString currentTimeStr = QString("%1:%2:%3").arg(hours,2,10, QChar('0')).arg(minutes,2,10, QChar('0')).arg(seconds,2,10, QChar('0'));
        ui->timeLabel->setText(currentTimeStr + " / "+ durationStr);
    }
}

void VideoTool::init()
{
    startTime = av_gettime();
    stopTime = startTime;
    resumeTime = startTime;
    // 转换为小时、分钟和秒
    long long hours = allTime / AV_TIME_BASE /3600;
    long long minutes = (allTime /AV_TIME_BASE ) % 3600 / 60;
    long long seconds = allTime /AV_TIME_BASE % 60;
    durationStr = QString("%1:%2:%3").arg(hours,2,10, QChar('0')).arg(minutes,2,10, QChar('0')).arg(seconds,2,10, QChar('0'));
    ui->timeLabel->setText("00:00:00 / "+ durationStr);
    emit sigVolumn(audioSlider->value());
    ui->timeSlider->setValue(0);
    isPlay = true;
    if(!isPlay){
        ui->pauseButton->toggle();
    }
}


void VideoTool::on_timeSlider_sliderPressed()
{
    mutex.lock();
    isChanging = true;
    mutex.unlock();
}


void VideoTool::on_timeSlider_valueChanged(int value)
{
    if(isChanging){
        int64_t delay = value / 1000.0f * allTime;
        ui->timeSlider->setValue(delay*1000/allTime);

        long long hours = delay / AV_TIME_BASE /3600;
        long long minutes = (delay /AV_TIME_BASE ) % 3600 / 60;
        long long seconds = delay /AV_TIME_BASE % 60;
        QString currentTimeStr = QString("%1:%2:%3").arg(hours,2,10, QChar('0')).arg(minutes,2,10, QChar('0')).arg(seconds,2,10, QChar('0'));
        ui->timeLabel->setText(currentTimeStr + " / "+ durationStr);
    }
}

void VideoTool::reduceTime() {
    ui->leftButton->click();
}

void VideoTool::addTime() {
    ui->rightButton->click();
}

void VideoTool::setMoveTimeInterval(int second) {
    timeInterval = second;
}

