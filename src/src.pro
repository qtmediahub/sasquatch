include(common.pri)

linux*: include(qtsingleapplication/qtsingleapplication.pri)

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
    qml-extensions/actionmapper.cpp \
    qml-extensions/mediaplayerhelper.cpp \
    dataproviders/proxymodel.cpp \
    dataproviders/playlist.cpp \
    plugins/mediamodel.cpp \
    plugins/mediainfo.cpp

QT += declarative script network

HEADERS += qmh-config.h \
    global.h \
    frontend.h \
    backend.h \
    plugins/qmhplugininterface.h \
    plugins/qmhplugin.h \
    dataproviders/proxymodel.h \
    dataproviders/dirmodel.h \
    qml-extensions/qmlfilewrapper.h \
    qml-extensions/actionmapper.h \
    qml-extensions/mediaplayerhelper.h \
    dataproviders/playlist.h \
    plugins/mediamodel.h \
    plugins/mediainfo.h

linux* {
    message(Linux specific options: (*default unless adjusted in src.pro))
    message(avahi*)
    message(glviewport*)
    CONFIG += avahi
}

CONFIG += glviewport

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

!avahi {
    DEFINES += QMH_NO_AVAHI 
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


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}
