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
private:
    void initClass();

    void initStyle();
private slots:
    void updatePosition(qint64 position);
    

    void updateDuration(qint64 duration);
    

    void updateState(QMediaPlayer::PlaybackState state);
    
    void slot_on_toolButton_Click();

    void slot_treeView_clicked(const QModelIndex &index);

private:
    Ui::MyMusicPlayer* ui;

    void resizeEvent(QResizeEvent* event);

    void paintEvent(QPaintEvent* event);

    bool eventFilter(QObject* watched, QEvent* event);

    QMediaPlayer* m_player;
    QStandardItemModel* m_model;
    QTimer* m_timer;

    //喇叭音量滑动条

    //喇叭音量
    QAudioOutput audioOutput;

    //记录当前音乐
    QString m_currentMusic;

    //记录歌曲时长
    qint64 m_durationTime;
};

#endif // LEFTSIDEBARBUTTON_H
