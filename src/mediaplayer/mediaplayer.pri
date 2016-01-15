vlc {
    DEFINES += MEDIAPLAYER_VLC
    HEADERS += $$PWD/mediaplayervlc.h
    SOURCES += $$PWD/mediaplayervlc.cpp
    LIBS += -lvlc
} else:!no-dbus {
    DEFINES += MEDIAPLAYER_DBUS
    HEADERS += $$PWD/mediaplayerdbus.h
} else {
    HEADERS += $$PWD/mediaplayerdummy.h
}
