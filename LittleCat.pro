QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += $$files($$PWD/*.cpp) \
    $$files($$PWD/Component/*.cpp,true) \
    $$files($$PWD/Component/musicPlayer/*.cpp,true)

HEADERS += $$files($$PWD/*.h) \
    $$files($$PWD/Component/*.h,true)
    $$files($$PWD/Component/musicPlayer/*.h,true)


FORMS += $$files($$PWD/*.ui) \
    $$files($$PWD/Component/*.ui,true)
    $$files($$PWD/Component/musicPlayer/*.ui,true)
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
