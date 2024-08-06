
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "Component/leftsidebarbutton.h"
#include <QVBoxLayout>
#include <QSplitter>
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

    downloadSoft *downlaodTool;

    QVBoxLayout *leftLayout,*rightLayout ,* middleLayout;

    QSplitter *mainMenuSplitter;

    QWidget *innerWidget;

	MyMusicPlayer* mymusicplayer;

    VideoWidget *videoPlayer;

    DecodeThread *decode;

    OpenGLWidget *openGLWidget;

    PlayVideoThread *videoThread;
    PlayAudioThread *audioThread;

    VideoList *videoList;


    void init();

protected:

    bool eventFilter(QObject *watched, QEvent *event) override;

    void paintRect();

private slots:
    void playFile(QString file);

    void stopPlayVideo();

    void pauseVideo(bool ok);

    void exitFullScreen();

    void setTypeList(int style);
};

#endif // MAINWIDGET_H
