QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network openglwidgets 3dcore multimedia


CONFIG += c++17

INCLUDEPATH += $$PWD/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += $$files($$PWD/src/*.cpp) \
    $$files($$PWD/src/Component/*.cpp,true) \
    $$files($$PWD/src/Component/musicPlayer/*.cpp,true)

HEADERS += $$files($$PWD/src/*.h) \
    $$files($$PWD/src/Component/*.h,true)\
    $$files($$PWD/src/Component/musicPlayer/*.h,true) \
    $$files($$PWD/include/libavformat/*.h,true)\
    $$files($$PWD/include/libavcodec/*.h,true)\
    $$files($$PWD/include/libavutil/*.h,true)\
    $$files($$PWD/include/libavfilter/*.h,true)\
    $$files($$PWD/include/libswscale/*.h,true)\
    $$files($$PWD/include/libswresample/*.h,true)\
    $$files($$PWD/include/libavdevice/*.h,true)\
    $$files($$PWD/include/libpostproc/*.h,true)
FORMS += $$files($$PWD/src/*.ui) \
    $$files($$PWD/src/Component/*.ui,true)\
    $$files($$PWD/src/Component/musicPlayer/*.ui,true)
CONFIG += lrelease
CONFIG += embed_translations


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    source/resource.qrc

LIBS += -L$$PWD/lib -lassimp-vc143-mt -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
