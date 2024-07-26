#ifndef MAINFROM_H
#define MAINFROM_H

#include <QWidget>
#include "http.h"


namespace Ui {
class MainFrom;
}


class MainFrom : public QWidget
{
    Q_OBJECT

public:
    explicit MainFrom(QWidget *parent = nullptr);
    ~MainFrom();

private slots:
    void on_btn_down_clicked();

    void onDownloadProgress(qint64 cur, qint64 total);

private:
    Ui::MainFrom *ui;

    Http m_http;
    qint64 m_bytes;
    bool m_status;
};

#endif // MAINFROM_H
