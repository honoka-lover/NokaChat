#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <qtimer.h>
#include <QTime>
#include <qdebug.h>
#include <QTreeView>

#pragma execution_character_set("utf-8")

MyMusicPlayer::MyMusicPlayer(QWidget *parent)
	:QWidget(parent)
	,ui(new Ui::MyMusicPlayer)
	,m_model(nullptr)
    ,m_currentMusic("")
	,m_durationTime(1)
{
	ui->setupUi(this);

	initClass();
	initStyle();
}

MyMusicPlayer::~MyMusicPlayer()
{
	delete ui;
	delete m_player;
	delete m_model;
	delete m_timer;
}

void MyMusicPlayer::playerMusic(QString file)
{
	auto* player = new QMediaPlayer;
	player->setSource(QUrl::fromLocalFile(file));//"F:/code/python/pyHonoka/vanilla/111.mp3"
	auto audioOutput = new QAudioOutput();
	player->setAudioOutput(audioOutput);
	audioOutput->setVolume(50);
	//    player->setLoopCount(-1);

	//    connect(player, &QMediaPlayer::positionChanged, [=](QAudio::State state) {
	//        if (state == QMediaPlayer::StoppedState) {
	//            qDebug() << "Media playback finished!";
	//        }
	//    });

	player->play();


}

void MyMusicPlayer::addRow(QStringList qstrList)
{
	QList<QStandardItem*> itemList;
	for (auto str : qstrList) {
		QStandardItem* item = new QStandardItem();
		item->setText(str);
		itemList.append(item);
	}
	m_model->appendRow(itemList);
}

void MyMusicPlayer::addRow(const QString& fileName)
{
	QFileInfo fileInfo(fileName);
	QStandardItem* item = new QStandardItem(fileInfo.fileName());
	item->setData(fileName, Qt::UserRole);
	m_model->appendRow(item);
}

void MyMusicPlayer::play()
{
	m_player->play();

	ui->toolButton->setStyleSheet("border-image: url(:/source/png/start.png);");
}

void MyMusicPlayer::pause()
{
	m_player->pause();
	ui->toolButton->setStyleSheet("border-image: url(:/source/png/pause.png);");
}

void MyMusicPlayer::stop()
{
	m_player->stop();
	ui->toolButton->setStyleSheet("border-image: url(:/source/png/pause.png);");
}


void MyMusicPlayer::updatePosition(qint64 position)
{
	QTime time = QTime::fromMSecsSinceStartOfDay(position);
	QString timeString = time.toString("mm:ss");
	ui->firstTime->setText(timeString);
	ui->progressBar->setValue(100 * position / m_durationTime);
	
}

void MyMusicPlayer::updateDuration(qint64 duration)
{
	m_durationTime = duration;
	QTime time = QTime::fromMSecsSinceStartOfDay(duration);
	QString timeString = time.toString("mm:ss");
	ui->secondTime->setText(timeString);
}

void MyMusicPlayer::updateState(QMediaPlayer::PlaybackState state)
{
	switch (state) {
	case QMediaPlayer::PlayingState:
		ui->toolButton->setObjectName("musicPause");

		break;
	case QMediaPlayer::PausedState:
		ui->toolButton->setObjectName("musicPlay");

		break;
	case QMediaPlayer::StoppedState:
		ui->toolButton->setObjectName("musicPlay");

		break;
	}
}

void MyMusicPlayer::slot_on_toolButton_Click()
{
	if (ui->toolButton->property("state").toBool()) {
		if (!m_model->rowCount())
			return;

		if (m_currentMusic.isEmpty()) {
			qDebug() << "未选择音乐";
			return;
		}

		m_player->setSource(QUrl::fromLocalFile(m_currentMusic));

		play();
		qDebug() << "播放音乐";
		ui->toolButton->setProperty("state", false);
	}
	else {
		pause();
		qDebug() << "暂停音乐";
		ui->toolButton->setProperty("state", true);
	}
}

void MyMusicPlayer::slot_treeView_clicked(const QModelIndex &index)
{
    m_currentMusic = index.data(Qt::UserRole).toString();
	ui->toolButton->setProperty("state", true);
	ui->toolButton->click();
}


void MyMusicPlayer::initClass()
{
	//if (m_treeModel == nullptr) {
	//	m_treeModel = new QStandardItemModel(ui->treeView);
	//	ui->treeView->setModel(m_treeModel);
	//	ui->treeView->setHeaderHidden(true);
	//}
	ui->secondTime->setText("0:00");
	ui->frame->installEventFilter(this);
	connect(ui->toolButton, &QToolButton::clicked, this, &MyMusicPlayer::slot_on_toolButton_Click);

	m_player = new QMediaPlayer(this);
	m_model = new QStandardItemModel(this);
	ui->treeView->setModel(m_model);
    ui->treeView->installEventFilter(this);
	connect(m_player, &QMediaPlayer::positionChanged, this, &MyMusicPlayer::updatePosition);
	connect(m_player, &QMediaPlayer::durationChanged, this, &MyMusicPlayer::updateDuration);
	connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MyMusicPlayer::updateState);

	m_timer = new QTimer(this);
	//connect(m_timer, &QTimer::timeout, this, &MyMusicPlayer::updatePosition);
	m_timer->start(1000);
	
	QDir dir(".");
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	dir.setSorting(QDir::Name | QDir::Reversed);
	QFileInfoList fileList = dir.entryInfoList();

	for (const QFileInfo& fileInfo : fileList) {
		if (fileInfo.fileName().endsWith(".mp3"))
			addRow(fileInfo.filePath());
	}
    QModelIndex index = m_model->index(0, 0);
    m_currentMusic = index.data(Qt::UserRole).toString();


    connect(ui->treeView,&QTreeView::doubleClicked,this,&MyMusicPlayer::slot_treeView_clicked);


	//初始化按钮，记录是否弹起
	 ui->toolButton->setProperty("state", true);

	 //设置音量
	
	 m_player->setAudioOutput(&audioOutput);
	 audioOutput.setVolume(20);
}

void MyMusicPlayer::initStyle()
{
    ui->toolButton->setObjectName("musicPlay");

	ui->trumpetButton->setObjectName("trumpt");

	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->progressBar->setRange(0, 100);
	ui->progressBar->setValue(0);
	ui->progressBar->setTextVisible(false);
}

void MyMusicPlayer::resizeEvent(QResizeEvent * event)
{
}

void MyMusicPlayer::paintEvent(QPaintEvent* event)
{
}

bool MyMusicPlayer::eventFilter(QObject* watched, QEvent* event)
{
    // if (watched == ui->treeView && event->type() == QMouseEvent::MouseButtonPress) {
 //        QModelIndex index = m_model.;
 //        m_currentMusic = index.data(Qt::UserRole).toString();

    // 	return true;
 //    }else if(watched == ui->treeView && event->type() == QMouseEvent::GraphicsSceneMouseDoubleClick){

 //        return true;
 //    }
	return false;
}


