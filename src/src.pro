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
    frontend.cpp \
    backend.cpp \
    qmlextensions/customcursor.cpp \
    qmlextensions/dashboard.cpp \
    plugins/qmhplugin.cpp \
    dataproviders/foldermodel.cpp \
    dataproviders/thumbnailer.cpp

QT += declarative

HEADERS += frontend.h \
    backend.h \
    qmlextensions/customcursor.h \
    qmlextensions/dashboard.h \
    plugins/qmhplugininterface.h \
    plugins/qmhplugin.h \
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
