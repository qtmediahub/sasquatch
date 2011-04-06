TEMPLATE = app
TARGET = qmh
DESTDIR = ../../hub
DEPENDPATH += .
INCLUDEPATH += . ..
QT += network declarative

LIBS += -lqmhcore
QMAKE_LIBDIR += ../../hub

linux* {
    include(../3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

# Input
SOURCES += main.cpp
