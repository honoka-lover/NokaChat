#ifndef LEFTSIDEBARBUTTON_H
#define LEFTSIDEBARBUTTON_H

#include <QWidget>
#include <QResizeEvent>

namespace Ui {
class LeftSideBarButton;
}

class LeftSideBarButton : public QWidget
{
    Q_OBJECT
signals:
    void selectStyle(int);

public:
    explicit LeftSideBarButton(QWidget *parent = nullptr);
    ~LeftSideBarButton();

    void InitItemStyle();

private slots:
    void on_listWidget_clicked(const QModelIndex &index);

private:
    Ui::LeftSideBarButton *ui;

    void resizeEvent(QResizeEvent *event);

    void paintEvent(QPaintEvent *event);

    bool eventFilter(QObject *watched,QEvent *event);


};

#endif // LEFTSIDEBARBUTTON_H
