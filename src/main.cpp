
#include "mainwidget.h"

#include <QApplication>


#include <QSslSocket>
#include <iostream>
#include <QMediaDevices>
#include <QOffscreenSurface>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序图标
    QIcon icon(":/png/icon.png");
    a.setWindowIcon(icon);
    //ssl版本
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    //查询版本
    {
        QOffscreenSurface surf;
        surf.create();

        QOpenGLContext ctx;
        ctx.create();
        ctx.makeCurrent(&surf);

        GLint major, minor;
        ctx.functions()->glGetIntegerv(GL_MAJOR_VERSION, &major); //gl大版本（整型）
        ctx.functions()->glGetIntegerv(GL_MINOR_VERSION, &minor); //gl小版本（整型）

        qDebug() << "OpenGL Version Info:" << (const char*)ctx.functions()->glGetString(GL_VERSION);
        qDebug() << "OpenGL Version Major:" << major << "OpenGL Version Minor:" << minor;

        ctx.doneCurrent();
    }
    MainWidget w;
    w.show();

    return a.exec();
}




