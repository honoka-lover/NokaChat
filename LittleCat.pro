QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Component/downloadsoft.cpp \
    Component/http.cpp \
    Component/leftsidebarbutton.cpp \
    Component/mainfrom.cpp \
    main.cpp \
    mainwidget.cpp

HEADERS += \
    Component/downloadsoft.h \
    Component/http.h \
    Component/leftsidebarbutton.h \
    Component/mainfrom.h \
    mainwidget.h

FORMS += \
    Component/downloadsoft.ui \
    Component/leftsidebarbutton.ui \
    Component/mainfrom.ui \
    mainwidget.ui

TRANSLATIONS += \
    Vanilla_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
