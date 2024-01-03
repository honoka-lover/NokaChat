#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <qtimer.h>
#include <QTime>
#include <qdebug.h>

#pragma execution_character_set("utf-8")

MyMusicPlayer::MyMusicPlayer(QWidget *parent)
	:QWidget(parent)
	,ui(new Ui::MyMusicPlayer)
	,m_model(nullptr)
{
	ui->setupUi(this);

	initClass();
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
	if (!m_model->rowCount())
		return;

	QModelIndex index = m_model->index(0, 0);
	QString fileName = index.data(Qt::UserRole).toString();
	m_player->setSource(QUrl::fromLocalFile(fileName));

	auto audioOutput = new QAudioOutput();
	m_player->setAudioOutput(audioOutput);
	audioOutput->setVolume(50);
	m_player->play();
}

void MyMusicPlayer::pause()
{
	m_player->pause();
}

void MyMusicPlayer::stop()
{
	m_player->stop();
}


void MyMusicPlayer::updatePosition(qint64 position)
{
	QTime time = QTime::fromMSecsSinceStartOfDay(position);
	QString timeString = time.toString("mm:ss");
	ui->firstTime->setText(timeString);
}

void MyMusicPlayer::updateDuration(qint64 duration)
{
	QTime time = QTime::fromMSecsSinceStartOfDay(duration);
	QString timeString = time.toString("mm:ss");
	ui->secondTime->setText(timeString);
}

void MyMusicPlayer::updateState(QMediaPlayer::PlaybackState state)
{
	switch (state) {
	case QMediaPlayer::PlayingState:
		ui->toolButton->setText("Pause");
		break;
	case QMediaPlayer::PausedState:
		ui->toolButton->setText("Play");
		break;
	case QMediaPlayer::StoppedState:
		ui->toolButton->setText("Play");
		break;
	}
}

void MyMusicPlayer::slot_on_toolButton_Click()
{
	QDir dir(".");
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	dir.setSorting(QDir::Name | QDir::Reversed);
	QFileInfoList fileList = dir.entryInfoList();

	for (const QFileInfo& fileInfo : fileList) {
		if (fileInfo.fileName().endsWith(".mp3"))
			addRow(fileInfo.filePath());
	}
	play();
	qDebug() << u8"播放音乐";
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
	connect(m_player, &QMediaPlayer::positionChanged, this, &MyMusicPlayer::updatePosition);
	connect(m_player, &QMediaPlayer::durationChanged, this, &MyMusicPlayer::updateDuration);
	connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MyMusicPlayer::updateState);

	m_timer = new QTimer(this);
	//connect(m_timer, &QTimer::timeout, this, &MyMusicPlayer::updatePosition);
	m_timer->start(1000);
	
}

void MyMusicPlayer::resizeEvent(QResizeEvent * event)
{
}

void MyMusicPlayer::paintEvent(QPaintEvent* event)
{
}

bool MyMusicPlayer::eventFilter(QObject* watched, QEvent* event)
{
	return false;
}
