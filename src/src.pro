DESTDIR = ../hub
TEMPLATE = app
TARGET = qmh
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
    dataproviders/foldermodel.cpp \
    dataproviders/thumbnailer.cpp \
    dataproviders/proxymodel.cpp \
    dataproviders/modelindexiterator.cpp

QT += declarative script network

HEADERS += config.h \
    frontend.h \
    backend.h \
    plugins/qmhplugininterface.h \
    plugins/qmhplugin.h \
    dataproviders/foldermodel.h \
    dataproviders/thumbnailer.h \
    dataproviders/proxymodel.h \
    dataproviders/dirmodel.h \
    dataproviders/modelindexiterator.h \
    qml-extensions/qmlfilewrapper.h

glgs {
    CONFIG += gl
    message(Using the OpenGL graphics system: I hope you know what you are doing)
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

#for() structure does not work with lupdate
TRANSLATIONS = $$system(cat $$DESTDIR/supported_languages | while read i; do echo translations/"$i".ts; done)
