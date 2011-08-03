include(common.pri)

DESTDIR = ../hub/lib
TEMPLATE = lib
TARGET = qmhcore
DEPENDPATH += .

QMAKE_CFLAGS_RELEASE += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CFLAGS_DEBUG += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CXXFLAGS += -fvisibility=hidden  -fvisibility-inlines-hidden

DEFINES += BUILDING_QMH

# Input
SOURCES += qmh-config.cpp \
    frontend.cpp \
    backend.cpp \
    actionmapper.cpp \
    trackpad.cpp \
    scopedtransaction.cpp \
    skin.cpp \
    device.cpp \
    dbreader.cpp \
    devicemanager.cpp \
    powermanager.cpp \
    widgetwrapper.cpp \
    customcursor.cpp \
    qmh-util.cpp \
    qml-utils.cpp

HEADERS += qmh-config.h \
    global.h \
    frontend.h \
    backend.h \
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
    widgetwrapper.h \
    qmh-util.h \
    customcursor.h \
    qml-utils.h

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
