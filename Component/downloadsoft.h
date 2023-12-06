#ifndef DOWNLOADSOFT_H
#define DOWNLOADSOFT_H

#include <QWidget>
#include "multithreaddownload.h"

namespace Ui {
class downloadSoft;
}

class downloadSoft : public QWidget
{
    Q_OBJECT

public:
    explicit downloadSoft(QWidget *parent = nullptr);
    ~downloadSoft() override;
    void StartDownload(QString url);

private slots:
    void on_toolButton_clicked();

private:
    Ui::downloadSoft *ui;

    DownloadControl *downloadControl;
};

#endif // DOWNLOADSOFT_H
