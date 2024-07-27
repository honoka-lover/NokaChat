#ifndef VIDEOTOOL_H
#define VIDEOTOOL_H

#include <QWidget>
#include <QSlider>
#include <QTimer>
#include <QMutex>
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
    void updateTimeStamp(int64_t);
public:
    explicit VideoTool(QWidget *parent = nullptr);
    ~VideoTool();

    void setAllTime(int64_t time);

    void setPauseButtonToggle();

    void reduceTime();

    void addTime();

    void setMoveTimeInterval(int second);
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

    void upDateSlider();
    void on_timeSlider_sliderPressed();

    void on_timeSlider_valueChanged(int value);

private:
    Ui::VideoTool *ui;

    int64_t allTime;
    int64_t startTime;
    int64_t stopTime;
    int64_t resumeTime;
    QString durationStr = "";
    //喇叭音量滑动条
    QSlider *audioSlider;

    QTimer *m_timer;

    //前进后退的时间间隔
    int timeInterval = 30;

    bool isPlay = false;
    bool isChanging = false;
    QMutex mutex;

    void init();

};

#endif // VIDEOTOOL_H
