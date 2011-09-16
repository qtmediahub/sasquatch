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
    declarativeview.cpp \
    libraryinfo.cpp \
    file.cpp \
    abstractmediaplayer.cpp \
    tarfileengine.cpp \
    processmanager.cpp

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
    refcountedengine.h \
    processmanager.h

!scenegraph {
    HEADERS += \
        declarativeview.h
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

unix:!symbian {
    target.path = $${PREFIX}/lib/
    INSTALLS += target
}


