#include "videolist.h"
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QSettings>
#include <QToolButton>
#include <QFileDialog>
VideoList::VideoList(QWidget* parent):
    QWidget(parent)
{
    m_view = new QTreeView;
    layout = new QVBoxLayout;
    layout1 = new QHBoxLayout;
    pathLabel = new QLabel;
    openButton = new QToolButton;
    innerWidget = new QWidget;
    layout1->addWidget(pathLabel);
    layout1->addWidget(openButton);
    innerWidget->setLayout(layout1);
    layout->addWidget(innerWidget);


    extensions << ".mp4" << ".mp3";

    m_view->setModel(&m_model);
    m_view->setHeaderHidden(true);
    m_view->setEditTriggers(QTreeView::EditTrigger::NoEditTriggers);

    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_view);
    this->setLayout(layout);


    // 创建 QSettings 对象，指定 INI 文件的路径
    QSettings settings("setting.ini", QSettings::IniFormat);

    // 读取 General 组中的键值
    QString path = settings.value("Video/ListPath").toString();
    setDir(path,extensions);
    m_view->setObjectName("VideoList");
    connect(m_view,&QTreeView::doubleClicked,this,&VideoList::playVideo);

    openButton->setText("选择文件夹");
    connect(openButton,&QToolButton::clicked,this,&VideoList::selectDir);
}

void VideoList::addItem(QStandardItem *item)
{

}

void VideoList::setDir(QString path ,QStringList typeList)
{
    m_path = path;
    clearTree();
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
    pathLabel->setText(m_path);
    pathLabel->setToolTip(m_path);
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

void VideoList::clearTree()
{
    QList<QStandardItem*> childItems = m_model.invisibleRootItem()->takeRow(0); // 获取第一个子项并将其从模型中移除
    for (QStandardItem *item : childItems) {
        delete item; // 删除子项
    }
}

void VideoList::selectDir() {
    QString folderName = QFileDialog::getExistingDirectory(this, "Select Folder");
    if (!folderName.isEmpty()) {
        setDir(folderName,extensions);
    }
}
