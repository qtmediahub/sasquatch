include(common.pri)

DESTDIR = ../lib
TEMPLATE = lib
TARGET = qmhcore
DEPENDPATH += .

QMAKE_CFLAGS_RELEASE += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CFLAGS_DEBUG += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CXXFLAGS += -fvisibility=hidden  -fvisibility-inlines-hidden

DEFINES += BUILDING_QMH QMH_LIB

# Input
SOURCES += \
    skinruntime.cpp \
    mediaserver.cpp \
    actionmapper.cpp \
    trackpad.cpp \
    scopedtransaction.cpp \
    skin.cpp \
    device.cpp \
    dbreader.cpp \
    devicemanager.cpp \
    powermanager.cpp \
    customcursor.cpp \
    libraryinfo.cpp \
    file.cpp \
    abstractmediaplayer.cpp \
    processmanager.cpp \
    skinmanager.cpp \
    settings.cpp \
    globalsettings.cpp \
    appsmanager.cpp \
    pushqml.cpp \
    ipaddressfinder.cpp \
    mainwindow.cpp

!qt5 {
    SOURCES += \
        tarfileengine.cpp \
        inputcontext.cpp
    HEADERS += \
        tarfileengine.h \
        inputcontext.h
}

HEADERS += \
    global.h \
    skinruntime.h \
    mediaserver.h \
    dirmodel.h \
    file.h \
    actionmapper.h \
    trackpad.h \
    scopedtransaction.h \
    skin.h \
    device.h \
    dbreader.h \
    devicemanager.h \
    powermanager.h \
    customcursor.h \
    libraryinfo.h \
    abstractmediaplayer.h \
    refcountcache.h \
    processmanager.h \
    skinmanager.h \
    declarativeview.h \
    mediaplayerdummy.h \
    settings.h \
    globalsettings.h \
    appsmanager.h \
    pushqml.h \
    ipaddressfinder.h \
    mainwindow.h \
    metrics.h

qt5 {
    SOURCES += \
        sgview.cpp \
        libraryinfo_qt5.cpp
} else {
    SOURCES += \
        declarativeview.cpp \
        libraryinfo_qt4.cpp
}

vlc {
    DEFINES += MEDIAPLAYER_VLC
    HEADERS += mediaplayervlc.h
    SOURCES += mediaplayervlc.cpp
    LIBS += -lvlc
} else:!no-dbus {
    DEFINES += MEDIAPLAYER_DBUS
    HEADERS += mediaplayerdbus.h
}

include(rpc/rpc.pri)
include(media/media.pri)
include(httpserver/httpserver.pri)

#for() structure does not work with lupdate
TRANSLATIONS = $$system(cat $$DESTDIR/supported_languages | while read i; do echo translations/"$i".ts; done)

avahi {
    DEFINES += QMH_AVAHI
    # avahi support
    include(3rdparty/libqavahi/libqavahi.pri)
    INCLUDEPATH += 3rdparty/libqavahi/
} else {
    SOURCES += staticservicebrowsermodel.cpp
    HEADERS += staticservicebrowsermodel.h
}

!no-dbus {
    QT += dbus
    SOURCES += devicemanagerdbus.cpp
    HEADERS += devicemanagerdbus.h
}

target.path = $${PREFIX}/lib/
INSTALLS += target











