#include "videotool.h"
#include "ui_videotool.h"
#include <QTime>
#include <QDebug>
#include <QTimer>
VideoTool::VideoTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoTool)
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
    init();
}

void VideoTool::setPauseButtonState(bool ok)
{
    if(ok){
        qDebug()<<"pause";
        ui->pauseButton->setObjectName("musicPause");
    }else{
        qDebug()<<"play";
        ui->pauseButton->setObjectName("musicPlay");

    }
    ui->pauseButton->setStyleSheet(ui->pauseButton->styleSheet());
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

}


void VideoTool::on_leftButton_clicked()
{

}


void VideoTool::on_timeSlider_sliderReleased()
{

}


void VideoTool::on_pauseButton_toggled(bool checked)
{
    if(checked){
        qDebug()<<"pause";
        ui->pauseButton->setObjectName("musicPause");
    }else{
        qDebug()<<"play";
        ui->pauseButton->setObjectName("musicPlay");

    }
    ui->pauseButton->setStyleSheet(ui->pauseButton->styleSheet());
    emit sigPuase(checked);
}

void VideoTool::setVolumn()
{
    auto value = audioSlider->value();
    emit sigVolumn(value);
}

void VideoTool::init()
{
    startTime = av_gettime();
    // 转换为小时、分钟和秒
    long long hours = allTime / AV_TIME_BASE /3600;
    long long minutes = (allTime /AV_TIME_BASE ) % 3600 / 60;
    long long seconds = allTime /AV_TIME_BASE % 60;
    durationStr = QString("%1:%2:%3").arg(hours,2,10, QChar('0')).arg(minutes,2,10, QChar('0')).arg(seconds,2,10, QChar('0'));
    ui->timeLabel->setText("00:00:00 / "+ durationStr);
    emit sigVolumn(audioSlider->value());
}

