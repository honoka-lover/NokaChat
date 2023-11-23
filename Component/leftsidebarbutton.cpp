#include "leftsidebarbutton.h"
#include "ui_leftsidebarbutton.h"
#include <QFile>
#include <QTimer>
LeftSideBarButton::LeftSideBarButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeftSideBarButton)
{
    ui->setupUi(this);

    //初始化样式表
    QFile file(QString(":/myStyle.qss"));
    file.open(QFile::ReadOnly);
    this->setStyleSheet(file.readAll());
//    this->setObjectName("mainUI");
    file.close();

    ui->frame->setObjectName("back");
    ui->frame_2->setObjectName("back");
    ui->frame_3->setObjectName("back");


    QTimer::singleShot(100,[=](){

        InitItemStyle();

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


}

void LeftSideBarButton::resizeEvent(QResizeEvent *event)
{
    for(int i = 0;i < ui->listWidget->count(); i++){
        auto item = ui->listWidget->item(i);
        item->setSizeHint(QSize(ui->listWidget->width(),ui->listWidget->height()/6));
    }
}
