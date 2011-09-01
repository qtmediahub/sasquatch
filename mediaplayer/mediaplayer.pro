include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
TARGET = ../hub/mediaplayer
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

TEMP_DIR = $$PWD/.tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
RCC_DIR = $$TEMP_DIR/.rcc

include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)

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

unix:!symbian {
    maemo5 {
        target.path = $$PREFIX/bin
    } else {
        target.path = $$PREFIX/bin
    }
    INSTALLS += target
}
