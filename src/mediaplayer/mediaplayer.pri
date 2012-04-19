vlc {
    DEFINES += MEDIAPLAYER_VLC
    HEADERS += $$PWD/mediaplayervlc.h
    SOURCES += $$PWD/mediaplayervlc.cpp
    LIBS += -lvlc
} else: 7425 {
message("7425 mediaplayer integrated")
    DEFINES += MEDIAPLAYER_7425
    HEADERS += $$PWD/mediaplayer7425.h
    SOURCES += $$PWD/mediaplayer7425.cpp
} else: ST7105 {
    message("ST7105 mediaplayer integrated")
    DEFINES += MEDIAPLAYER_ST7105
    HEADERS += $$PWD/mediaplayerST7105.h
    SOURCES += $$PWD/mediaplayerST7105.cpp
} else: ST7108 {
    message("ST7108 mediaplayer integrated")
    DEFINES += MEDIAPLAYER_ST7108
    HEADERS += $$PWD/mediaplayerST7108.h
    SOURCES += $$PWD/mediaplayerST7108.cpp
} else: ST7540 {
    message("ST7540 mediaplayer integrated")
    DEFINES += MEDIAPLAYER_ST7540
    HEADERS += $$PWD/mediaplayerST7540.h
    SOURCES += $$PWD/mediaplayerST7540.cpp
} else: TRIDENT_SHINER_GSTTSPLAYER {
    message("Tridents Shiner's GstTsPlayer mediaplayer integrated")
    DEFINES += MEDIAPLAYER_TRIDENT_SHINER_GSTTSPLAYER
    HEADERS += $$PWD/mediaplayerTridentShinerGstTsPlayer.h
    SOURCES += $$PWD/mediaplayerTridentShinerGstTsPlayer.cpp
} else: TRIDENT_SHINER_MINIPLAYER {
    message("Tridents Shiner's miniplayer mediaplayer integrated")
    DEFINES += MEDIAPLAYER_TRIDENT_SHINER_MINIPLAYER
    HEADERS += $$PWD/mediaplayerTridentShinerMiniplayer.h
    SOURCES += $$PWD/mediaplayerTridentShinerMiniplayer.cpp
} else:!no-dbus {
    DEFINES += MEDIAPLAYER_DBUS
    HEADERS += $$PWD/mediaplayerdbus.h
} else {
    HEADERS += mediaplayerdummy.h
}
