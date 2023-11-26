#ifndef DOWNLOADSOFT_H
#define DOWNLOADSOFT_H

#include <QWidget>

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

private:
    Ui::downloadSoft *ui;
};

#endif // DOWNLOADSOFT_H
