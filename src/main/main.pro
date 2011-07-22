include($$PROJECTROOT/src/common.pri)
include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
TARGET = qmh
DESTDIR = $${PROJECTROOT}/hub
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

TEMP_DIR = $$PWD/.tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
RCC_DIR = $$TEMP_DIR/.rcc

linux* {
    include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

QT += sql

# Input
SOURCES += main.cpp

RESOURCES = main.qrc
