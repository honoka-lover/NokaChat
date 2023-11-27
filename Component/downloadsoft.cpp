#include <QUrl>
#include <QFile>
#include <QPen>
#include "downloadsoft.h"
#include "ui_downloadsoft.h"


downloadSoft::downloadSoft(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::downloadSoft)
{
    ui->setupUi(this);
}

downloadSoft::~downloadSoft()
{
    delete ui;
}
void downloadSoft::StartDownload(QString m_downloadUrl)
{

}

