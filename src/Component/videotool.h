#ifndef VIDEOTOOL_H
#define VIDEOTOOL_H

#include <QWidget>
#include <QSlider>
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
namespace Ui {
class VideoTool;
}

class VideoTool : public QWidget
{
    Q_OBJECT
signals:
    void sigPuase(bool);
    void sigVolumn(int);
public:
    explicit VideoTool(QWidget *parent = nullptr);
    ~VideoTool();

    void setAllTime(int64_t time);

    void setPauseButtonState(bool ok);

protected:
    bool eventFilter(QObject *watched,QEvent*event);

    void hideEvent(QHideEvent *event);

    void resizeEvent(QResizeEvent *event);
private slots:

    void on_rightButton_clicked();

    void on_leftButton_clicked();

    void on_timeSlider_sliderReleased();

    void on_pauseButton_toggled(bool checked);

    void setVolumn();

private:
    Ui::VideoTool *ui;

    int64_t allTime;
    int64_t startTime;
    QString durationStr = "";
    //喇叭音量滑动条
    QSlider *audioSlider;

    void init();

};

#endif // VIDEOTOOL_H
