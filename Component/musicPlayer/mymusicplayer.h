#ifndef MYMUSICPLAYER_H
#define MYMUSICPLAYER_H

#include <QWidget>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QMediaPlayer>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QTime>
#include <QTimer>
#include <QKeyEvent>
#include <qaudiooutput.h>
#include <qslider.h>
#include <atomic>
#include <mutex>
namespace Ui {
    class MyMusicPlayer;
}

class MyMusicPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MyMusicPlayer(QWidget* parent = nullptr);
    ~MyMusicPlayer();

    void playerMusic(QString file = "F:/code/python/pyHonoka/vanilla/111.mp3");

    void addRow(QStringList qstrList);

    void addRow(const QString& fileName);

    void play();
    void pause();
    void stop();

signals :
    //更新音乐进度条
//    void updateMusicSlider(qint64 position);

private:
    void initClass();

    void initStyle();

    void initConnect();

    //音量条点击
    void horizontalSliderPress();

    //音量条释放
    void horizontalSliderRelease();
private slots:
    void updatePosition(qint64 position);
    
    void updateDuration(qint64 duration);
    
    void updateState(QMediaPlayer::PlaybackState state);
    
    //播放按钮点击
    void slot_on_toolButton_Click();
    
    //树点击
    void slot_treeView_clicked(const QModelIndex &index);

    //音量设置
    void slot_setAudioValue(int i);

    //更新音乐进度条
    void slot_updateMusicSlider();
private:
    Ui::MyMusicPlayer* ui;

    void resizeEvent(QResizeEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    QMediaPlayer* m_player;
    QStandardItemModel* m_model;
    QTimer* m_timer;

    //喇叭音量滑动条
    QSlider *audioSlider;

	//喇叭音量
    QAudioOutput* audioOutput;

    //记录当前音乐
    QString m_currentMusic;

    //记录歌曲时长
    qint64 m_durationTime;

    //锁
    std::mutex sliderLock;
    
    //记录音乐条是否处于按下状态
    std::atomic<bool> audioSliderState;
};

#endif // LEFTSIDEBARBUTTON_H
