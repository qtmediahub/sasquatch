DESTDIR = ../hub
TEMPLATE = app
TARGET = qtmediahub
DEPENDPATH += .
INCLUDEPATH += .
TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

QMAKE_LIBDIR += ../hub
LIBS += -lqmhplugin
QMAKE_LFLAGS += -Wl,--rpath=.

# Input
SOURCES += main.cpp \
    frontend.cpp \
    backend.cpp \
    qmlextensions/customcursor.cpp \
    dataproviders/foldermodel.cpp \
    dataproviders/thumbnailer.cpp

QT += declarative

HEADERS += frontend.h \
    backend.h \
    qmlextensions/customcursor.h \
    dataproviders/foldermodel.h \
    dataproviders/thumbnailer.h

glgs {
    CONFIG += gl
    message(Using the OpenGL graphics system)
    DEFINES += GLGS
}

glviewport {
    CONFIG += gl
    message(Using an OpenGL viewport)
    DEFINES += GLVIEWPORT
}

gl {
    DEFINES += GL
    QT += opengl
} else {
    message(Not using GL acceleration)
}
