#ifndef VIDEOTOOL_H
#define VIDEOTOOL_H

#include <QWidget>
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

public:
    explicit VideoTool(QWidget *parent = nullptr);
    ~VideoTool();

    void setAllTime(int64_t time);
private slots:
    void on_pauseButton_clicked();

    void on_rightButton_clicked();

    void on_leftButton_clicked();

    void on_timeSlider_sliderReleased();

private:
    Ui::VideoTool *ui;

    int64_t allTime;
    int64_t startTime;
    QString durationStr = "";
};

#endif // VIDEOTOOL_H
