#include "http.h"

Http::Http(QObject *parent) : QObject(parent),
    m_netWorkManager(nullptr),
    m_reply(nullptr),
    m_fileSize(0),
    m_statusCode(0),
    m_downLoadedBytes(0),
    m_currentLoadedBytes(0),
    m_fileName(""),
    m_url(""),
    m_IsDownloading(false)
{   
    m_netWorkManager = new QNetworkAccessManager();
}

Http::~Http()
{
    if(m_reply)
    {
        m_reply->deleteLater();
        m_reply=nullptr;
    }
}

void Http::stopDownLoad()
{
    if(m_reply == nullptr) return;

    disconnect(m_reply,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    disconnect(m_reply,&QNetworkReply::errorOccurred,this,&Http::onError);
    disconnect(m_reply,SIGNAL(finished()),this,SLOT(onfinishedRequest()));

    m_reply->abort();
    m_reply->deleteLater();

    m_file.close();
    getCurrentFileSize();

    m_IsDownloading = false;
}

void Http::getCurrentFileSize()
{
    QFileInfo fileInfo("./"+m_fileName);
    if(fileInfo.exists())
    {
        m_currentLoadedBytes = fileInfo.size();
    }
    else
    {
        m_currentLoadedBytes = 0;
    }
}

void Http::startDownLoad(const QString &url)
{
    m_url = url;
    if(m_url.isEmpty()) return;

    if(!m_IsDownloading)
    {
        //获取请求头
        QNetworkRequest request;
        QUrl url = QUrl(m_url);
        request.setUrl(url);
        m_fileName = url.fileName();
        qDebug()<<"fileName = "<<m_fileName;
        m_reply = m_netWorkManager->head(request);
        m_state = requestHead;
        getCurrentFileSize();
        connect(m_reply,SIGNAL(finished()),this,SLOT(onfinishedRequest()));
    }
}

void Http::onfinishedRequest()
{
    if(m_reply==nullptr) return;

    if(m_state == requestHead)
    {       
        m_fileSize = m_reply->rawHeader("Content-Length").toInt();
        qDebug()<<"m_fileSize = "<<m_fileSize;
        if(m_currentLoadedBytes == m_fileSize)
        {
            qDebug()<<"文件已经存在!";
            return;
        }
        //https://down.qq.com/qqweb/PCQQ/PCQQ_EXE/PCQQ2020.exe
        QNetworkRequest request;
        request.setUrl(m_url);

        QEventLoop *loop = new QEventLoop;
        connect(m_netWorkManager, SIGNAL(finished(QNetworkReply*)), loop, SLOT(quit()));
        m_reply = m_netWorkManager->get(request);
        loop->exec();

        //获取状态码
        m_statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"statusCode="<<m_statusCode;
        m_state = requestBody;
    }

    QNetworkRequest request;
    if(m_statusCode==200)
    {
        request.setUrl(m_url);
    }
    else if(m_statusCode == 302)    //存在转调url
    {
        //获取实际下载地址
        QUrl realUrl = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        request.setUrl(realUrl);
    }
    else
    {
        return;
    }

    QString downLoadSize = QString::number(m_fileSize);
    QString selectSize = QString("bytes=%1-%2").arg(m_currentLoadedBytes).arg(downLoadSize);
    request.setRawHeader("Range",selectSize.toLatin1());
    m_reply = m_netWorkManager->get(request);

    connect(m_reply,SIGNAL(finished()),this,SLOT(onfinishedRequest()));
    connect(m_reply,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    connect(m_reply,&QNetworkReply::errorOccurred,this,&Http::onError);
}

void Http::onReadyRead()
{
    if(m_reply==nullptr) return;

    if(!m_file.isOpen())
    {
        m_file.setFileName("./"+m_fileName);
        m_file.open(QIODevice::WriteOnly|QIODevice::Append);
    }

    m_file.write(m_reply->readAll());

    m_downLoadedBytes =m_file.size();

    emit this->fileDownloadProgress(m_downLoadedBytes, m_fileSize);
    if(m_file.size() == m_fileSize)
    {
        qDebug()<<"download finished!";
        stopDownLoad();
    }
}

void Http::onError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);

    if(m_reply==nullptr) return;

    qDebug()<<"error:"<<m_reply->errorString();
}
