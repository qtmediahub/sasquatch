include(common.pri)

DESTDIR = ../hub
TEMPLATE = app
TARGET = qmh
DEPENDPATH += .
INCLUDEPATH += .

# Input
SOURCES += main.cpp \
    qmh-config.cpp \
    frontend.cpp \
    backend.cpp \
    rpcapi.cpp \
    qml-extensions/actionmapper.cpp \
    dataproviders/proxymodel.cpp \
    dataproviders/modelindexiterator.cpp \
    dataproviders/playlist.cpp \
    plugins/mediamodel.cpp \
    plugins/mediainfo.cpp

QT += declarative script network

HEADERS += qmh-config.h \
    global.h \
    frontend.h \
    backend.h \
    rpcapi.h \
    plugins/qmhplugininterface.h \
    plugins/qmhplugin.h \
    dataproviders/proxymodel.h \
    dataproviders/dirmodel.h \
    dataproviders/modelindexiterator.h \
    qml-extensions/qmlfilewrapper.h \
    qml-extensions/actionmapper.h \
    dataproviders/playlist.h \
    plugins/mediamodel.h \
    plugins/mediainfo.h

include(rpc/rpc.pri)

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

mac {
    QT += webkit\
          multimedia
          
    CONFIG += mobility

    MOBILITY += multimedia\
                systeminfo
}

#for() structure does not work with lupdate
TRANSLATIONS = $$system(cat $$DESTDIR/supported_languages | while read i; do echo translations/"$i".ts; done)
include(delaysymresolution.pri)

# avahi support
include(3rdparty/libqavahi/libqavahi.pri)
INCLUDEPATH += 3rdparty/libqavahi/

