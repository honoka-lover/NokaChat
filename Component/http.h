#ifndef HTTP_H
#define HTTP_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QFile>
#include <QEventLoop>
#include <QFileInfo>
#include <QThread>
#include <QCoreApplication>

enum Status
{
    requestHead=0,
    requestBody,
};

class Http : public QObject
{
    Q_OBJECT
public:
    explicit Http(QObject *parent = nullptr);
    ~Http();
public:
    void startDownLoad(const QString &url);
    void stopDownLoad();

private:
    void getCurrentFileSize();

signals:
    void fileDownloadProgress(qint64 cur, qint64 total);

private slots:
    void onfinishedRequest();
    void onReadyRead();
    void onError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_netWorkManager;
    QNetworkReply *m_reply;

    int m_fileSize;
    int m_statusCode;
    qint64 m_downLoadedBytes;
    qint64 m_currentLoadedBytes;

    QString m_fileName;
    QString m_url;
    Status m_state;

    bool m_IsDownloading;
    QFile m_file;
};

#endif // HTTP_H
