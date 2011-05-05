include(common.pri)

DESTDIR = ../hub/lib
TEMPLATE = lib
TARGET = qmhcore
DEPENDPATH += .
INCLUDEPATH += . ./plugins/qmhplugin

#global options
CONFIG += glviewport
#end global options

#platform options
linux* {
    message(Linux specific options: (*default unless adjusted in src.pro))
    message(glviewport*)
}

scenegraph {
    DEFINES += SCENEGRAPH
}

mac {
    QT += webkit\
          multimedia
          
    CONFIG += mobility

    MOBILITY += multimedia\
                systeminfo
}

# This is needed for Maemo5 to recognize minimization of the application window
# Thanks quit coding!
maemo5 {
    QT += dbus
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}
#end platform options

#painting options
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
#end painting options

# Input
SOURCES += qmh-config.cpp \
    frontend.cpp \
    backend.cpp \
    qml-extensions/actionmapper.cpp \
    qml-extensions/trackpad.cpp \
    dataproviders/proxymodel.cpp \
    dataproviders/playlist.cpp \
    scopedtransaction.cpp \
    dbreader.cpp \
    skin.cpp

QT += declarative script network sql

HEADERS += qmh-config.h \
    global.h \
    frontend.h \
    backend.h \
    qmhplugin.h \
    dataproviders/proxymodel.h \
    dataproviders/dirmodel.h \
    qml-extensions/qmlfilewrapper.h \
    qml-extensions/actionmapper.h \
    qml-extensions/trackpad.h \
    dataproviders/playlist.h \
    scopedtransaction.h \
    dbreader.h \
    skin.h

include(rpc/rpc.pri)
include(media/media.pri)

#for() structure does not work with lupdate
TRANSLATIONS = $$system(cat $$DESTDIR/supported_languages | while read i; do echo translations/"$i".ts; done)

avahi {
    DEFINES += QMH_AVAHI
    # avahi support
    include(3rdparty/libqavahi/libqavahi.pri)
    INCLUDEPATH += 3rdparty/libqavahi/
}
