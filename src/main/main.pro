TEMPLATE = app
TARGET = qmh
DESTDIR = $${PROJECTROOT}/hub
DEPENDPATH += .
INCLUDEPATH += /src/
QT += network declarative

LIBS += -lqmhcore
QMAKE_LIBDIR += $${PROJECTROOT}/hub/lib

linux* {
    include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

# Input
SOURCES += main.cpp
