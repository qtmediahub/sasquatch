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
    dataproviders/foldermodel.cpp \
    dataproviders/thumbnailer.cpp \
    dataproviders/proxymodel.cpp

QT += declarative script

HEADERS += config.h \
    frontend.h \
    backend.h \
    plugins/qmhplugininterface.h \
    plugins/qmhplugin.h \
    dataproviders/foldermodel.h \
    dataproviders/thumbnailer.h \
    dataproviders/proxymodel.h \
    dataproviders/dirmodel.h

glgs {
    CONFIG += gl
    message(Using the OpenGL graphics system)
    DEFINES += GLGS
}

glviewport {
    CONFIG += gl
    message(Let Donald know if you see text vanishing, see docs/known_issues)
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
