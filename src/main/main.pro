TEMPLATE = app
TARGET = qmh
DESTDIR = $${PROJECTROOT}/hub
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

LIBS += -lqmhcore
QMAKE_LIBDIR += $${PROJECTROOT}/hub/lib

linux* {
    include($${PROJECTROOT}/src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
}

# Input
SOURCES += main.cpp

RESOURCES = main.qrc
