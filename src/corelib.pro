include(common.pri)

DESTDIR = ../lib
TEMPLATE = lib
TARGET = qmhcore
DEPENDPATH += .

QMAKE_CFLAGS_RELEASE += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CFLAGS_DEBUG += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CXXFLAGS += -fvisibility=hidden  -fvisibility-inlines-hidden

DEFINES += BUILDING_QMH

# Input
SOURCES += qmh-config.cpp \
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
    tarfileengine.cpp \
    processmanager.cpp \
    skinmanager.cpp \
    inputcontext.cpp

HEADERS += qmh-config.h \
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
    tarfileengine.h \
    refcountcache.h \
    processmanager.h \
    skinmanager.h \
    declarativeview.h \
    inputcontext.h

qt5 {
    SOURCES += \
        sgview.cpp
} else {
    SOURCES += \
        declarativeview.cpp
}

!no-dbus {
    HEADERS += mediaplayer_dbus.h
    SOURCES += mediaplayer_dbus.cpp
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

contains(QT_CONFIG, dbus) {
    QT += dbus
    SOURCES += devicemanagerdbus.cpp
    HEADERS += devicemanagerdbus.h
}

vlc {
    SUBDIRS += vlc
    LIBS += -lvlc
}


target.path = $${PREFIX}/lib/
INSTALLS += target



