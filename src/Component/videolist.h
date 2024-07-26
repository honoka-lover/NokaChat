#ifndef VIDEOLIST_H
#define VIDEOLIST_H

#include <QTreeView>
#include <QWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileInfo>
#include <QStyledItemDelegate>
#include <QFontMetrics>
class VideoList : public QWidget
{
    Q_OBJECT
signals:
    void sigFileName(QString);

public:
    VideoList(QWidget *parent = nullptr);

    void addItem(QStandardItem *item);

    void setDir(QString path,QStringList type);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void playVideo(const QModelIndex& index);
private:
    QTreeView* m_view;
    QStandardItemModel m_model;
    QString m_path;

    void flushTree();
    QVBoxLayout *layout;
};

#endif // VIDEOLIST_H
