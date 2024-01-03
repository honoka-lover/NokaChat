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


private slots:
    void updatePosition(qint64 position);
    

    void updateDuration(qint64 duration);
    

    void updateState(QMediaPlayer::PlaybackState state);
    
    void slot_on_toolButton_Click();
private:
    Ui::MyMusicPlayer* ui;

    void resizeEvent(QResizeEvent* event);

    void paintEvent(QPaintEvent* event);

    bool eventFilter(QObject* watched, QEvent* event);

    QMediaPlayer* m_player;
    QStandardItemModel* m_model;
    QTimer* m_timer;
};

#endif // LEFTSIDEBARBUTTON_H