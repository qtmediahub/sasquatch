include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
mac:{
    DESTDIR = $$PROJECTROOT
} else {
    DESTDIR = $$PROJECTROOT/bin
}
TARGET=qtmediahub
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

TEMP_DIR = $$PWD/.tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
RCC_DIR = $$TEMP_DIR/.rcc

Qt += gui

qt5 {
    Qt += quick
} else {
    Qt += declarative
    include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

# Input
SOURCES += main.cpp \
           mainwindow.cpp

HEADERS += mainwindow.h

RESOURCES = main.qrc

target.path = $$PREFIX/bin
INSTALLS += target

