
#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QFile>
#include "QApplication"
#include <QPainter>
#include "QTimer"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , leftUI(nullptr)
    , mainFrom(nullptr)
{
    ui->setupUi(this);

    leftUI = new LeftSideBarButton();
    LeftLayout.addWidget(leftUI);
    LeftLayout.setSpacing(0);
    LeftLayout.setContentsMargins(0,0,0,0);
    ui->frame->setLayout(&LeftLayout);


    auto dir = QApplication::applicationDirPath();
//    QFile file(QString(dir+"/../myStyle.qss"));
    QFile file(QString(":/myStyle.qss"));
    file.open(QFile::ReadOnly);
    this->setStyleSheet(file.readAll());

    QString styleStr = dir+"/../source/png/1.jpg";
    qDebug()<<styleStr;

//    ui->frame_2->setStyleSheet(QString("border-image:url(%1);").arg(styleStr));

    ui->frame_2->setStyleSheet(file.readAll());
    ui->frame_2->setObjectName("mainUI");
    file.close();

    ui->frame_2->installEventFilter(this);


    mainFrom = new MainFrom(ui->frame_2);
//    mainFrom->show();

}

MainWidget::~MainWidget()
{
    delete ui;
}

bool MainWidget::eventFilter(QObject *watched, QEvent *event) {
    if(watched == ui->frame_2 && event->type() == QEvent::Paint)//发生绘图事件，且是在widget上发生的
    {
        paintRect();
        QPainter painter(ui->frame_2);

        // 设置画笔样式
        QPen pen(Qt::red);
        pen.setWidth(3);
        painter.setPen(pen);

        // 绘制圆形
        QRectF rectangle(10.0, 20.0, 80.0, 80.0);    // 圆形所在矩形位置和大小
        painter.drawEllipse(rectangle);

        return true;
    }
    else
        return false;

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


