DESTDIR = ../hub
TEMPLATE = app
TARGET = qtmediahub
DEPENDPATH += .
INCLUDEPATH += .
TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc


# Input
SOURCES += main.cpp \
    config.cpp \
    frontend.cpp \
    backend.cpp \
    qmlextensions/customcursor.cpp \
    dataproviders/foldermodel.cpp \
    dataproviders/thumbnailer.cpp \
    dataproviders/proxymodel.cpp

QT += declarative

HEADERS += config.h \
    frontend.h \
    backend.h \
    qmlextensions/customcursor.h \
    plugins/qmhplugininterface.h \
    plugins/qmhplugin.h \
    dataproviders/foldermodel.h \
    dataproviders/thumbnailer.h \
    dataproviders/proxymodel.h

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
