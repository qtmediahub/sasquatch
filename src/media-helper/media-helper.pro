include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
TARGET = ../../hub/media-helper
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
SOURCES += main.cpp mediabackendinterface.cpp
HEADERS += mediabackendinterface.h

unix:!symbian {
    maemo5 {
        target.path = $$PREFIX/bin
    } else {
        target.path = $$PREFIX/bin
    }
    INSTALLS += target
}
