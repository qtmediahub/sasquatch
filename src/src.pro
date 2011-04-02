include(common.pri)

DESTDIR = ../hub
TEMPLATE = app
TARGET = qmh
DEPENDPATH += .
INCLUDEPATH += . ./plugins/qmhplugin

#global options
CONFIG += glviewport
#end global options

#platform options
linux* {
    include(qtsingleapplication/qtsingleapplication.pri)
    message(Linux specific options: (*default unless adjusted in src.pro))
    message(glviewport*)
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
SOURCES += main.cpp \
    qmh-config.cpp \
    frontend.cpp \
    backend.cpp \
    qml-extensions/actionmapper.cpp \
    qml-extensions/mediaplayerhelper.cpp \
    qml-extensions/trackpad.cpp \
    dataproviders/proxymodel.cpp \
    dataproviders/playlist.cpp \
    plugins/mediamodel.cpp \
    plugins/mediainfo.cpp \
    skin.cpp

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
    qml-extensions/trackpad.h \
    dataproviders/playlist.h \
    plugins/qmhplugin/qmhplugin.h \
    plugins/mediamodel.h \
    plugins/mediainfo.h \
    skin.h

include(rpc/rpc.pri)

#for() structure does not work with lupdate
TRANSLATIONS = $$system(cat $$DESTDIR/supported_languages | while read i; do echo translations/"$i".ts; done)
include(delaysymresolution.pri)

avahi {
    DEFINES += QMH_AVAHI
    # avahi support
    include(3rdparty/libqavahi/libqavahi.pri)
    INCLUDEPATH += 3rdparty/libqavahi/
}

QMAKE_LIBDIR += $$PROJECTROOT/hub/plugins
LIBS += -lqmhplugin
QMAKE_LFLAGS += -Wl,--rpath=$$PROJECTROOT/hub/plugins
