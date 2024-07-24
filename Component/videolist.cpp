#include "videolist.h"
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QSettings>
VideoList::VideoList(QWidget* parent):
    QWidget(parent)
{
    m_view = new QTreeView(this);
    layout = new QVBoxLayout(this);

    QStringList extensions;
    extensions << ".mp4" << ".mp3";

    m_view->setModel(&m_model);
    m_view->setHeaderHidden(true);
    m_view->setEditTriggers(QTreeView::EditTrigger::NoEditTriggers);

    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_view);
    this->setLayout(layout);


    // 创建 QSettings 对象，指定 INI 文件的路径
    QSettings settings("../source/setting.ini", QSettings::IniFormat);

    // 读取 General 组中的键值
    QString path = settings.value("Video/ListPath").toString();
    setDir(path,extensions);
    m_view->setObjectName("VideoList");
    connect(m_view,&QTreeView::doubleClicked,this,&VideoList::playVideo);
}

void VideoList::addItem(QStandardItem *item)
{

}

void VideoList::setDir(QString path ,QStringList typeList)
{
    m_path = path;
    QDir directory(m_path);
    if (!directory.exists()) {
        qWarning() << "Directory does not exist.";
        return;
    }
    // 只列出文件，不包括子目录
    QStringList files = directory.entryList(QDir::Files);

    // 打印文件名
    foreach (const QString &fileName, files) {
        foreach (const QString &type,typeList){
            if(fileName.endsWith(type)){
                QStandardItem *item = new QStandardItem;
                item->setText(fileName);
                item->setToolTip(fileName);
                item->setData(m_path + '/' + fileName);
                m_model.insertRow(0,item);
            }
        }
    }
    flushTree();
}

void VideoList::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void VideoList::playVideo(const QModelIndex &index)
{
    QStandardItem* item = m_model.itemFromIndex(index);
    emit sigFileName(item->data().toString());
}

void VideoList::flushTree()
{

}
