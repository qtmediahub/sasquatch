include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
mac:{
    DESTDIR = $$PROJECTROOT
} else {
    DESTDIR = $$PROJECTROOT/bin
}
TARGET = pushui
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

TEMP_DIR = $$PWD/.tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
RCC_DIR = $$TEMP_DIR/.rcc

Qt += gui

# Input
SOURCES += main.cpp \
    mainwindow.cpp

target.path = $$PREFIX/bin
INSTALLS += target

HEADERS += \
    mainwindow.h



