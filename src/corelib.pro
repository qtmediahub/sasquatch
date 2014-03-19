include(common.pri)

DESTDIR = $$BUILDROOT/hub/lib
TEMPLATE = lib
TARGET = qmhcore
DEPENDPATH += .

QMAKE_CFLAGS_RELEASE += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CFLAGS_DEBUG += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CXXFLAGS += -fvisibility=hidden  -fvisibility-inlines-hidden

DEFINES += BUILDING_QMH

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
    mainwindow.cpp \
    deviceexposure.cpp \
    contextcontentrpc.cpp

HEADERS += \
    global.h \
    skinruntime.h \
    mediaserver.h \
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
    settings.h \
    globalsettings.h \
    appsmanager.h \
    pushqml.h \
    ipaddressfinder.h \
    mainwindow.h \
    metrics.h \
    deviceexposure.h \
    inputnotifier.h \
    contextcontentrpc.h

qt5 {
    SOURCES += \
        sgview.cpp \
        libraryinfo_qt5.cpp \
        inputnotifier_qt5.cpp
} else {
    SOURCES += \
        declarativeview.cpp \
        libraryinfo_qt4.cpp \
        inputnotifier_qt4.cpp
}

#HEADERS += dirmodel.h

include(rpc/rpc.pri)
include(media/media.pri)
include(mediaplayer/mediaplayer.pri)
include(httpserver/httpserver.pri)

INCLUDEPATH += mediaplayer

#for() structure does not work with lupdate
TRANSLATIONS = $$system(cat $$DESTDIR/supported_languages | while read i; do echo translations/"$i".ts; done)

staticserivce {
    DEFINES += QMH_STATIC_SERVICE_BROWSER
    SOURCES += staticservicebrowsermodel.cpp
    HEADERS += staticservicebrowsermodel.h
} else {
    SOURCES += simpleservicebrowsermodel.cpp
    HEADERS += simpleservicebrowsermodel.h
}

!no-dbus {
    QT += dbus
    SOURCES += devicemanagerdbus.cpp
    HEADERS += devicemanagerdbus.h
}

target.path = $${PREFIX}/lib/
INSTALLS += target

