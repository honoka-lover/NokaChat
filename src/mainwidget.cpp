
#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QFile>
#include "QApplication"
#include <QPainter>
#include "QTimer"
#include "Component/videowidget.h"
#include <QToolButton>
MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , leftUI(nullptr)
    , mainFrom(nullptr)
    , mymusicplayer(nullptr)
    , openGLWidget(nullptr)
    , decode(nullptr)
    , audioThread(nullptr)
    , videoThread(nullptr)
{
    ui->setupUi(this);

    init();

    QFile file(QString(":/myStyle.qss"));
    file.open(QFile::ReadOnly);
    this->setStyleSheet(file.readAll());
    file.close();

    setComponentVisible();


    connect(videoList,&VideoList::sigFileName,this,&MainWidget::playFile);
    connect(videoPlayer,&VideoWidget::pauseScreen,this,&MainWidget::pauseVideo);
    connect(videoPlayer,&VideoWidget::exitFullScreen,this,&MainWidget::exitFullScreen);

    ui->frame_2->installEventFilter(this);
    setMouseTracking(true);
    this->installEventFilter(this);
    videoPlayer->grabKeyboard();
}

MainWidget::~MainWidget()
{
    if(decode){
        decode->stop();
        decode->wait();
        delete decode;
    }
    if(videoThread) {
        videoThread->stop();
        videoThread->wait();
        delete videoThread;
    }
    if(audioThread){
        audioThread->stop();
        audioThread->wait();
        delete audioThread;
    }
    if(videoPlayer){
        delete videoPlayer;
    }
    delete ui;
}

void MainWidget::setComponentVisible()
{
    leftUI->hide();

    downlaodTool->hide();

    mymusicplayer->hide();
    // videoPlayer->hide();
    if(openGLWidget)
        openGLWidget->hide();

//    videoList->hide();
}

void MainWidget::init()
{
    ui->frame->setMinimumWidth(400);
    ui->frame_2->setMinimumWidth(200);

    leftUI = new LeftSideBarButton();
    LeftLayout.addWidget(leftUI);
    LeftLayout.setSpacing(0);
    LeftLayout.setContentsMargins(0,0,0,0);
    RightLayout.setSpacing(0);
    RightLayout.setContentsMargins(0,0,0,0);

    downlaodTool = new downloadSoft(this);
    RightLayout.addWidget(downlaodTool);

    mymusicplayer = new MyMusicPlayer(this);
    RightLayout.addWidget(mymusicplayer);

    QSurfaceFormat format;
    //设置每个每个像素采样样本个数，用于抗锯齿
    format.setSamples(16);
    videoPlayer = new VideoWidget(this);
    videoPlayer->setFormat(format);
    videoPlayer->show();

    LeftLayout.addWidget(videoPlayer);


    openGLWidget = new OpenGLWidget(this);
    openGLWidget->setFormat(format);
    openGLWidget->StartAnimating();

    videoList = new VideoList(this);
    RightLayout.addWidget(videoList);

    LeftLayout.addWidget(openGLWidget);

    ui->frame->setLayout(&LeftLayout);
    ui->frame_2->setLayout(&RightLayout);

//    //设置右边背景图片
//    ui->frame_2->setObjectName("mainUI");
//
//    //设置背景图片
//    ui->frame->setObjectName("mainUI2");

    // ui->pushButton->setObjectName("slider");

//    if(mainFrom == nullptr){
//        mainFrom = new MainFrom(ui->frame_2);
//        mainFrom->hide();
//    }

}

bool MainWidget::eventFilter(QObject *watched, QEvent *event) {
    if(watched == ui->frame_2 && event->type() == QEvent::Paint)//发生绘图事件，且是在widget上发生的
    {
//        paintRect();
//        QPainter painter(ui->frame_2);

//        // 设置画笔样式
//        QPen pen(Qt::red);
//        pen.setWidth(3);
//        painter.setPen(pen);

//        // 绘制圆形
//        QRectF rectangle(10.0, 20.0, 80.0, 80.0);    // 圆形所在矩形位置和大小
//        painter.drawEllipse(rectangle);

        return false;
    }else if(watched == this && event->type() == QEvent::KeyPress){
        if(openGLWidget && openGLWidget->isVisible())
            openGLWidget->grabKeyboard();
        else if(videoPlayer && videoPlayer->isVisible())
            videoPlayer->grabKeyboard();
//         QKeyEvent *e = (QKeyEvent*)event;
//         switch(e->key()){
//         case Qt::Key_Space:
// //            qDebug()<<"暂停";
//             if(!isPlay){
//                 isPlay = true;
//                 if(decode)
//                     decode->resume();
//                 if(audioThread)
//                     audioThread->resume();
//                 if(videoThread)
//                     videoThread->resume();
//             }else {
//                 isPlay = false;
//                 if (decode)
//                     decode->pause();
//                 if (audioThread)
//                     audioThread->pause();
//                 if (videoThread)
//                     videoThread->pause();
//             }
//             break;
//         case Qt::Key_Escape:
//             qDebug()<<"退出全屏";
//             break;
//         }
    }


    return QObject::eventFilter(watched, event);
}

void MainWidget::paintRect() {
    QPainter painter(ui->frame_2);

    QPen pen; //画笔。绘制图形边线，由颜色、宽度、线风格等参数组成
    pen.setColor(QColor(255,0,0,255));
    QBrush brush;   //画刷。填充几何图形的调色板，由颜色和填充风格组成
    brush.setColor(QColor(0,255,0,120));
    brush.setStyle(Qt::SolidPattern);

    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawRect(50,50,200,100);
}

void MainWidget::playFile(QString file)
{
    if(decode){
        if(decode->getFileName() == file)
            return;
        decode->stop();
        decode->wait();
        videoThread->stop();
        videoThread->wait();
        audioThread->stop();
        audioThread->wait();
        delete decode;
        delete videoThread;
        delete audioThread;
    }
    decode = new DecodeThread(this);
    decode->bindVideoWidget(videoPlayer);
    decode->setFileName(file);
    // decode->setFileName("../NokaChat/日南結里,Yunomi - 白猫海賊船.mp3");
    // decode->setFileName("E:/NokaChat/source/test.wav");

    audioThread = new PlayAudioThread(this);
    videoThread = new PlayVideoThread(this);
    decode->bindPlayThread(audioThread,videoThread);
    connect(videoThread,&PlayVideoThread::frameDecoded,videoPlayer,&VideoWidget::setFrame);
    connect(videoPlayer,&VideoWidget::sendVolumn,audioThread,&PlayAudioThread::setVolumn);
    decode->start();
    audioThread->start();
    videoThread->start();
}

void MainWidget::pauseVideo(bool ok)
{
    if(!ok){
        if(decode)
            decode->resume();
        if(audioThread)
            audioThread->resume();
        if(videoThread)
            videoThread->resume();
    }else {
        if (decode)
            decode->pause();
        if (audioThread)
            audioThread->pause();
        if (videoThread)
            videoThread->pause();
    }

}

void MainWidget::exitFullScreen()
{
    LeftLayout.addWidget(videoPlayer);
}


