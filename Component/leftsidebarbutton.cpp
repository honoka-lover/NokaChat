#include "leftsidebarbutton.h"
#include "ui_leftsidebarbutton.h"
#include <QFile>
#include <QTimer>
#include "QLinearGradient"
#include "QPen"
#include "QPainter"
#include "QDebug"

LeftSideBarButton::LeftSideBarButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeftSideBarButton)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
//    this->setWindowOpacity(0.5);


//    ui->frame->setObjectName("back");
//    ui->frame_2->setObjectName("back");
//    ui->frame_3->setObjectName("back");


    QTimer::singleShot(100,[=](){

        InitItemStyle();
        ui->frame->installEventFilter(this);
    });
}

LeftSideBarButton::~LeftSideBarButton()
{
    delete ui;
}

void LeftSideBarButton::InitItemStyle()
{
    //隐藏滚动条
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    for(int i = 0;i < ui->listWidget->count(); i++){
        auto item = ui->listWidget->item(i);
        item->setSizeHint(QSize(ui->listWidget->width(),ui->listWidget->height()/6));
    }

    this->setStyleSheet("border-style:flat;");


}

void LeftSideBarButton::resizeEvent(QResizeEvent *event)
{
    for(int i = 0;i < ui->listWidget->count(); i++){
        auto item = ui->listWidget->item(i);
        item->setSizeHint(QSize(ui->listWidget->width(),ui->listWidget->height()/6));
    }
}

void LeftSideBarButton::paintEvent(QPaintEvent *event) {

}

bool LeftSideBarButton::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->frame && event->type() == QEvent::Paint){
        QLinearGradient gradient(0, 0, 0, height());
        gradient.setColorAt(0, QColor(0, 255, 0,100));
        gradient.setColorAt(1, QColor(0, 0, 255,100));

        QPainter painter(ui->frame);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(gradient);

        QPen p = painter.pen();
        p.setColor(Qt::transparent);
        painter.setPen(p);

//        painter.setCompositionMode( QPainter::CompositionMode_Clear );

        painter.drawRect(0, 0, width(), height());

        return true;
    }
    else{
        return false;
    }
}
