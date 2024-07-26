﻿
#include "mainwidget.h"

#include <QApplication>


#include <QSslSocket>
#include <iostream>
#include <QMediaDevices>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << QSslSocket::sslLibraryBuildVersionString();

    MainWidget w;
    w.show();

    return a.exec();
}



