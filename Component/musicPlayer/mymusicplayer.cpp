#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <qtimer.h>
#include <QTime>
#include <qdebug.h>
#include <QTreeView>

#pragma execution_character_set("utf-8")

using std::mutex;

MyMusicPlayer::MyMusicPlayer(QWidget *parent)
	: QWidget(parent), ui(new Ui::MyMusicPlayer), m_model(nullptr), m_currentMusic(""), m_durationTime(1), audioSlider(nullptr), audioSliderState(false)
{
	ui->setupUi(this);

	initClass();
	initStyle();
	initConnect();

	ui->frame->installEventFilter(this);
	ui->treeView->installEventFilter(this);
	ui->trumpetButton->installEventFilter(this);
	audioSlider->installEventFilter(this);
	ui->horizontalSlider->installEventFilter(this);
}

MyMusicPlayer::~MyMusicPlayer()
{
	delete ui;
    if(m_player)
	    delete m_player;
    if(m_model)
	    delete m_model;
    if(m_timer)
	    delete m_timer;
}

void MyMusicPlayer::initClass()
{
	// if (m_treeModel == nullptr) {
	//	m_treeModel = new QStandardItemModel(ui->treeView);
	//	ui->treeView->setModel(m_treeModel);
	//	ui->treeView->setHeaderHidden(true);
	// }
	ui->secondTime->setText("0:00");

	m_player = new QMediaPlayer(this);
	m_model = new QStandardItemModel(this);
	audioOutput = new QAudioOutput(this);
	m_player->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.2);
	ui->treeView->setModel(m_model);

	QDir dir(".");
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	dir.setSorting(QDir::Name | QDir::Reversed);
	QFileInfoList fileList = dir.entryInfoList();

	for (const QFileInfo &fileInfo : fileList)
	{
		if (fileInfo.fileName().endsWith(".mp3"))
			addRow(fileInfo.filePath());
	}

	// addRow("qrc:/source/music/日南結里,Yunomi - 白猫海賊船.mp3");
	// addRow("qrc:/source/music/池鱼 - 姑娘别哭泣.mp3");

	QModelIndex index = m_model->index(0, 0);
	m_currentMusic = index.data(Qt::UserRole).toString();

	// 初始化按钮，记录是否弹起
	ui->toolButton->setProperty("state", true);
	m_player->setSource(QUrl::fromLocalFile(m_currentMusic));

	// 设置音乐条
	if (audioSlider == nullptr)
	{
		audioSlider = new QSlider(ui->frame);
		// 跟踪滑块
		audioSlider->setTracking(true);
		audioSlider->setRange(0, 100);
		audioSlider->setValue(20);
		audioSlider->setObjectName("myAudio");
		audioSlider->hide();
	}
}

void MyMusicPlayer::initConnect()
{

	// 绑定播放按钮点击事件
	connect(ui->toolButton, &QToolButton::clicked, this, &MyMusicPlayer::slot_on_toolButton_Click);

	// 绑定树点击事件
	connect(ui->treeView, &QTreeView::doubleClicked, this, &MyMusicPlayer::slot_treeView_clicked);

	// 绑定音乐类信号事件
	connect(m_player, &QMediaPlayer::positionChanged, this, &MyMusicPlayer::updatePosition);
	connect(m_player, &QMediaPlayer::durationChanged, this, &MyMusicPlayer::updateDuration);
	connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MyMusicPlayer::updateState);

	// 绑定滑动条与音量设置
	connect(audioSlider, &QSlider::valueChanged, this, &MyMusicPlayer::slot_setAudioValue);

	//更新音乐进度条
	// connect(this,&MyMusicPlayer::updateMusicSlider,this,&MyMusicPlayer::slot_updateMusicSlider);
}

void MyMusicPlayer::initStyle()
{
	ui->toolButton->setObjectName("musicPlay");

	ui->trumpetButton->setObjectName("trumpt");

	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->horizontalSlider->setRange(0, 100);
	ui->horizontalSlider->setValue(0);
	ui->horizontalSlider->setObjectName("musicSlider");

	QTimer::singleShot(50, [=]()
					   { audioSlider->setGeometry(ui->trumpetButton->x(), ui->frame_2->y() + ui->trumpetButton->y() - 100, ui->trumpetButton->width(), 100); });
}

void MyMusicPlayer::playerMusic(QString file)
{
	auto *player = new QMediaPlayer;
	player->setSource(QUrl::fromLocalFile(file)); //"F:/code/python/pyHonoka/vanilla/111.mp3"
	auto audioOutput = new QAudioOutput();
	player->setAudioOutput(audioOutput);
	audioOutput->setVolume(0.5);
	player->play();
}

void MyMusicPlayer::addRow(QStringList qstrList)
{
	QList<QStandardItem *> itemList;
	for (auto str : qstrList)
	{
		QStandardItem *item = new QStandardItem();
		item->setText(str);
		itemList.append(item);
	}
	m_model->appendRow(itemList);
}

void MyMusicPlayer::addRow(const QString &fileName)
{
	QFileInfo fileInfo(fileName);
	QStandardItem *item = new QStandardItem(fileInfo.fileName());
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

    if(position == m_durationTime && m_durationTime != 0){
        ui->toolButton->setProperty("state", true);
        ui->toolButton->setStyleSheet("border-image: url(:/source/png/pause.png);");
        disconnect(m_timer, &QTimer::timeout, this, &MyMusicPlayer::slot_updateMusicSlider);
        delete m_timer;
        ui->horizontalSlider->setValue(0);
        ui->firstTime->setText("0:00");
    }
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
	switch (state)
	{
	case QMediaPlayer::PlayingState:
		m_timer = new QTimer(this);
        m_timer->setInterval(500);
        connect(m_timer, &QTimer::timeout, this, &MyMusicPlayer::slot_updateMusicSlider);
        m_timer->start();
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
	if (ui->toolButton->property("state").toBool())
	{
		if (!m_model->rowCount())
			return;

		if (m_currentMusic.isEmpty())
		{
			qDebug() << "未选择音乐";
			return;
		}

		play();
		qDebug() << "播放音乐";
		ui->toolButton->setProperty("state", false);
	}
	else
	{
		pause();
		qDebug() << "暂停音乐";
		ui->toolButton->setProperty("state", true);
	}
}

void MyMusicPlayer::slot_treeView_clicked(const QModelIndex &index)
{
	m_currentMusic = index.data(Qt::UserRole).toString();
	ui->toolButton->setProperty("state", true);
	m_player->setSource(QUrl::fromLocalFile(m_currentMusic));
	// m_player->setSource(QUrl::fromLocalFile("qrc:/source/music/池鱼 - 姑娘别哭泣.mp3"));
	ui->toolButton->click();

    ui->horizontalSlider->setValue(0);
}

void MyMusicPlayer::slot_setAudioValue(int i)
{
	int xx = audioSlider->value();

	audioOutput->setVolume(i / 100.0);
	m_player->setAudioOutput(audioOutput);
}

void MyMusicPlayer::slot_updateMusicSlider()
{
	std::lock_guard<mutex> lock(sliderLock);


	if (audioSliderState == false && m_durationTime != 0)
	{
        qint64 position{};
        QTime time = QTime::fromString(ui->firstTime->text(),"mm:ss");
        position = time.msecsSinceStartOfDay();
		ui->horizontalSlider->setValue(100 * (double(position) / (double)m_durationTime) +1);

		// 歌曲播放完按钮切换为暂停
		if (ui->horizontalSlider->value() == 100)
		{
			ui->toolButton->setProperty("state", true);
			ui->toolButton->setStyleSheet("border-image: url(:/source/png/pause.png);");
		}
	}
}

void MyMusicPlayer::horizontalSliderPress()
{
}

void MyMusicPlayer::horizontalSliderRelease()
{
	std::lock_guard<mutex> lock(sliderLock);
	audioSliderState = false;

	qint64 position;
	
	QTime time = QTime::fromString(ui->secondTime->text(),"mm:ss");
	qint64 maxPosition = time.msecsSinceStartOfDay();
	
	position = ui->horizontalSlider->value() /100.0 * maxPosition;
	m_player->setPosition(position);

    QTime timeStart;
    timeStart = QTime::fromMSecsSinceStartOfDay(position);
    ui->firstTime->setText(timeStart.toString("mm:ss"));

}

void MyMusicPlayer::resizeEvent(QResizeEvent *event)
{
	audioSlider->setGeometry(ui->trumpetButton->x(), ui->frame_2->y() + ui->trumpetButton->y() - 100, ui->trumpetButton->width(), 100);
}

void MyMusicPlayer::paintEvent(QPaintEvent *event)
{
}

bool MyMusicPlayer::eventFilter(QObject *watched, QEvent *event)
{

	if ((watched != ui->trumpetButton && watched != audioSlider) && event->type() == QEvent::HoverMove)
	{

		audioSlider->hide();
	}

	if (watched == ui->trumpetButton)
	{
		if (event->type() == QEvent::HoverEnter)
		{
			audioSlider->show();

			// qDebug() << "鼠标放置到音量按钮上";
			return false;
		}
	}
	else if (watched == audioSlider)
	{

		return false;
	}
	else if (watched == ui->horizontalSlider && event->type() == QMouseEvent::MouseButtonPress)
	{
		std::lock_guard<mutex> lock(sliderLock);
		audioSliderState = true;
		horizontalSliderPress();
		return false;
	}
	else if (watched == ui->horizontalSlider && event->type() == QMouseEvent::MouseButtonRelease)
	{
		
		horizontalSliderRelease();
	}



	return false;
}
