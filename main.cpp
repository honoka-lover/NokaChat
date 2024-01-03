
#include "mainwidget.h"

#include <QApplication>

#include <QLocale>
#include <QTranslator>
#include <QSslSocket>

#pragma  execution_character_set("utf-8")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << QSslSocket::sslLibraryBuildVersionString();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Vanilla_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWidget w;
    w.show();
    return a.exec();
}

