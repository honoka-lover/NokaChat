
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
    setTypeList(0);

    connect(videoList,&VideoList::sigFileName,this,&MainWidget::playFile);
    connect(videoPlayer,&VideoWidget::pauseScreen,this,&MainWidget::pauseVideo);
    connect(videoPlayer,&VideoWidget::exitFullScreen,this,&MainWidget::exitFullScreen);
    connect(leftUI, &LeftSideBarButton::selectStyle,this,&MainWidget::setTypeList);

    ui->frame_2->installEventFilter(this);
    this->installEventFilter(this);
    videoPlayer->grabKeyboard();
    this->setWindowTitle("Vanilla");
}

MainWidget::~MainWidget()
{ 
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
    if(decode){
        decode->stop();
        decode->wait();
        delete decode;
    }
    if(videoPlayer){
        delete videoPlayer;
    }
    delete ui;
}

void MainWidget::setComponentVisible()
{
    // leftUI->hide();

    downlaodTool->hide();

    mymusicplayer->hide();
    videoPlayer->hide();
    if(openGLWidget)
        openGLWidget->hide();

   // videoList->hide();
}

void MainWidget::init()
{
    ui->frame->setMinimumWidth(200);
    ui->frame_2->setMinimumWidth(200);

    leftLayout = new QVBoxLayout;
    rightLayout = new QVBoxLayout;
    middleLayout = new QVBoxLayout;
    mainMenuSplitter = new QSplitter;
    mainMenuSplitter->setContentsMargins(0,0,0,0);
    //设置水平拆分布局
    mainMenuSplitter->setOrientation(Qt::Horizontal);
    // 设置为不透明分隔器
    mainMenuSplitter->setOpaqueResize(true);
    mainMenuSplitter->setHandleWidth(5);
    leftLayout->addWidget(mainMenuSplitter);

    innerWidget = new QWidget;

    leftLayout->setSpacing(0);
    leftLayout->setContentsMargins(0,0,0,0);
    middleLayout->setSpacing(0);
    middleLayout->setContentsMargins(0,0,0,0);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0,0,0,0);

    leftUI = new LeftSideBarButton();
    mainMenuSplitter->addWidget(leftUI);
    mainMenuSplitter->addWidget(innerWidget);
    mainMenuSplitter->setSizes(QList<int>()<<200<<600<<600);

    //左边frame窗口
    QSurfaceFormat format;
    //设置每个每个像素采样样本个数，用于抗锯齿
    format.setSamples(16);
    videoPlayer = new VideoWidget(this);
    videoPlayer->setFormat(format);
    videoPlayer->show();
    middleLayout->addWidget(videoPlayer);

    //左边frame窗口
    QSurfaceFormat format1;
    format1.setOption(QSurfaceFormat::DebugContext);
    openGLWidget = new OpenGLWidget(this);
    openGLWidget->setFormat(format1);
    openGLWidget->StartAnimating();
    middleLayout->addWidget(openGLWidget);

    //右边frame窗口
    downlaodTool = new downloadSoft(this);
    rightLayout->addWidget(downlaodTool);

    //右边frame窗口
    mymusicplayer = new MyMusicPlayer(this);
    rightLayout->addWidget(mymusicplayer);

    //右边frame窗口
    videoList = new VideoList(this);
    rightLayout->addWidget(videoList);



    innerWidget->setLayout(middleLayout);
    ui->frame->setLayout(leftLayout);
    ui->frame_2->setLayout(rightLayout);
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

    audioThread = new PlayAudioThread(this);
    videoThread = new PlayVideoThread(this);
    decode->bindPlayThread(audioThread,videoThread);
    connect(videoThread,&PlayVideoThread::frameDecoded,videoPlayer,&VideoWidget::setFrame);
    connect(videoPlayer,&VideoWidget::sendVolumn,audioThread,&PlayAudioThread::setVolumn);
    connect(videoPlayer,&VideoWidget::updateTime,decode,&DecodeThread::seek);
    decode->start();
    audioThread->start();
    videoThread->start();
}

void MainWidget::stopPlayVideo()
{
    if(decode){
        decode->stop();
        decode->wait();
        videoThread->stop();
        videoThread->wait();
        audioThread->stop();
        audioThread->wait();
        delete decode;
        delete videoThread;
        delete audioThread;
        decode = nullptr;
        videoThread = nullptr;
        audioThread = nullptr;
    }
    videoPlayer->clearPage();
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
    middleLayout->addWidget(videoPlayer);
}

void MainWidget::setTypeList(int style)
{
    if(style == 0){
        videoPlayer->show();
        ui->frame_2->show();
        videoList->show();

        openGLWidget->hide();
        downlaodTool->hide();
        mymusicplayer->hide();


    }else if(style == 1){
        stopPlayVideo();
        videoPlayer->hide();
        ui->frame_2->hide();

        openGLWidget->show();
        downlaodTool->hide();
        mymusicplayer->hide();
    }
}


