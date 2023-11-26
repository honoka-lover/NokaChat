#include "mainfrom.h"
#include "ui_mainfrom.h"

MainFrom::MainFrom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainFrom)
{
    ui->setupUi(this);
    m_status = true;
    qDebug() << "OpenSSL Support:" << QSslSocket::supportsSsl();

    connect(&m_http,SIGNAL(fileDownloadProgress(qint64, qint64)),this,SLOT(onDownloadProgress(qint64, qint64)));
    ui->edit_url->setPlaceholderText("请输入下载地址");
    ui->edit_url->setText("http://download.qt.io/archive/qt/5.14/5.14.2/submodules/qtdeclarative-everywhere-src-5.14.2.tar.xz");
    ui->edit_url->setToolTip(ui->edit_url->text());
}

MainFrom::~MainFrom()
{
    delete ui;
}

void MainFrom::on_btn_down_clicked()
{
    if(m_status)
    {
        m_http.startDownLoad(ui->edit_url->text());
        ui->label->setText("网络请求中...");
    }
    else
    {
        m_http.stopDownLoad();
        ui->btn_down->setText("下载");
        ui->label->setText("已暂停");
        m_status = true;
    }

}

void MainFrom::onDownloadProgress(qint64 cur, qint64 total)
{   
    ui->btn_down->setText("暂停");
    m_status = false;

    QString totalSize = QString::number(total/(1024*1024)) + "M";
    QString currentSize = QString::number(cur/(1024*1024)) + "M";
    ui->label->setText("下载中..."+currentSize +"/"+totalSize);

    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(cur);

    if(cur == total)
    {
        ui->label->setText("下载完毕");
        ui->btn_down->setText("下载");
    }
}

