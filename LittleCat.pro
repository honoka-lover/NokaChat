QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network openglwidgets 3dcore multimedia


CONFIG += c++17

INCLUDEPATH += $$PWD/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += $$files($$PWD/*.cpp) \
    $$files($$PWD/Component/*.cpp,true) \
    $$files($$PWD/Component/musicPlayer/*.cpp,true) \
    Component/audioplay.cpp \
    Component/dataqueue.cpp \
    Component/decodethread.cpp \
    Component/videowidget.cpp

HEADERS += $$files($$PWD/*.h) \
    $$files($$PWD/Component/*.h,true)\
    $$files($$PWD/Component/musicPlayer/*.h,true) \
    Component/audioplay.h \
    Component/dataqueue.h \
    Component/decodethread.h \
    Component/videowidget.h \
    $$files($$PWD/include/libavforamt/*.h,true)\
    $$files($$PWD/include/libavcodec/*.h,true)\
    $$files($$PWD/include/libavutil/*.h,true)\
    $$files($$PWD/include/libavfilter/*.h,true)\
    $$files($$PWD/include/libswscale/*.h,true)\
    $$files($$PWD/include/libswresample/*.h,true)\
FORMS += $$files($$PWD/*.ui) \
    $$files($$PWD/Component/*.ui,true)\
    $$files($$PWD/Component/musicPlayer/*.ui,true)
CONFIG += lrelease
CONFIG += embed_translations


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

LIBS += -L$$PWD/lib -lassimp-vc143-mt -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
