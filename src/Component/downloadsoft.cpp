#include <QUrl>
#include <QFile>
#include <QPen>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "downloadsoft.h"
#include "ui_downloadsoft.h"


downloadSoft::downloadSoft(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::downloadSoft),
    downloadControl(nullptr)
{
    ui->setupUi(this);

    if(downloadControl == nullptr){
        downloadControl = new DownloadControl();

        connect(downloadControl,&DownloadControl::FileDownloadFinished,this,[=](){
            ui->progressBar->setValue(100);
            // ui->label->setText("下载完成");
        });
    }
}

downloadSoft::~downloadSoft()
{
    delete ui;
}
void downloadSoft::StartDownload(QString m_downloadUrl)
{

}


void downloadSoft::on_toolButton_clicked()
{
    if(ui->lineEdit->text().compare("")){
            downloadControl->StartFileDownload(ui->lineEdit->text(),1);
        }
}

