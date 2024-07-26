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

public:
    explicit LeftSideBarButton(QWidget *parent = nullptr);
    ~LeftSideBarButton();

    void InitItemStyle();

private:
    Ui::LeftSideBarButton *ui;

    void resizeEvent(QResizeEvent *event);

    void paintEvent(QPaintEvent *event);

    bool eventFilter(QObject *watched,QEvent *event);


};

#endif // LEFTSIDEBARBUTTON_H
