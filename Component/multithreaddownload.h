#ifndef MULTITHREADDOWNLOAD_H
#define MULTITHREADDOWNLOAD_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>
#include <QFile>
#include <QUrl>
#include <QString>
#include <QTimer>
#include <mutex>
#include <thread>
#include <iostream>

class Download : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager m_Qnam;
    QNetworkReply *m_Reply;
    QFile *m_File;

    const int m_Index;
    qint64 m_HaveDoneBytes;
    qint64 m_StartPoint;
    qint64 m_EndPoint;

public:
    Download(int index, QObject *parent = 0);
    void StartDownload(const QUrl url, QFile *file,
                       qint64 startPoint=0, qint64 endPoint=-1);
signals:
    void DownloadFinished();

public slots:
    void FinishedSlot();
    void HttpReadyRead();
};

//用于管理文件的下载
class DownloadControl : public QObject
{
    Q_OBJECT
private:
    int m_DownloadCount;
    int m_FinishedNum;
    int m_FileSize;
    QUrl m_Url;
    QFile *m_File;
public:
    DownloadControl(QObject *parent = 0);
    void StartFileDownload(const QString &url, int count);
    qint64 GetFileSize(QUrl url);
signals:
    void FileDownloadFinished();
private slots:
    void SubPartFinished();
};


class MultithreadDownload : public QObject
{
    Q_OBJECT
public:
    explicit MultithreadDownload(QObject *parent = nullptr);

signals:
};

#endif // MULTITHREADDOWNLOAD_H
