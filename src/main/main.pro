include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
TARGET = ../../hub/qtmediahub
#DESTDIR = $${PROJECTROOT}/hub
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

unix:!symbian {
    maemo5 {
        target.path = $$PREFIX/bin
    } else {
        target.path = $$PREFIX/bin
    }
message(target.path)
    INSTALLS += target
}
