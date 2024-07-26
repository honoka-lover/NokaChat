
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "Component/leftsidebarbutton.h"
#include <QVBoxLayout>
#include "Component/mainfrom.h"
#include "Component/downloadsoft.h"
#include "Component/musicPlayer/mymusicplayer.h"
#include "Component/OpenGLWidget.h"
#include "Component/videowidget.h"
#include "Component/decodethread.h"
#include "Component/videolist.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class MainWidget : public QWidget

{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;

    LeftSideBarButton *leftUI;

    MainFrom *mainFrom;

    downloadSoft *downlaodTool;

    QVBoxLayout LeftLayout,RightLayout;

	MyMusicPlayer* mymusicplayer;

    VideoWidget *videoPlayer;

    DecodeThread *decode;

    OpenGLWidget *openGLWidget;

    PlayVideoThread *videoThread;
    PlayAudioThread *audioThread;

    VideoList *videoList;

    bool isPlay = false;
    void setComponentVisible();

    void init();

protected:

    bool eventFilter(QObject *watched, QEvent *event) override;

    void paintRect();

private slots:
    void playFile(QString file);

};

#endif // MAINWIDGET_H
