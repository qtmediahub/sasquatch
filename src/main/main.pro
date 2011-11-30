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

!qt5 {
    include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

Qt += gui declarative

# Input
SOURCES += main.cpp \
           mainwindow.cpp \
           skinselector.cpp

HEADERS += mainwindow.h \
           skinselector.h

RESOURCES = main.qrc

target.path = $$PREFIX/bin
INSTALLS += target

