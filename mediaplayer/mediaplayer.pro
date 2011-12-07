include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
TARGET = ../bin/mediaplayer
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

TEMP_DIR = $$PWD/.tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
RCC_DIR = $$TEMP_DIR/.rcc

qt5 {
    QT += quick
} else {
    include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

QT -= gui
QT += dbus

# Input
SOURCES += \
    main.cpp

HEADERS += \
    testingplayer.h

xine:unix: !no-pkg-cfg:system(pkg-config --exists libxine) {
    SOURCES += xineplayer.cpp
    HEADERS += xineplayer.h

    CONFIG += link_pkgconfig
    PKGCONFIG += libxine
    message(Use system libxine)
    DEFINES += XINE_PLAYER
} else {
    xine: message(Trying to compile libxine but pkgconfig does not know of it)
}

target.path = $$PREFIX/bin
INSTALLS += target
