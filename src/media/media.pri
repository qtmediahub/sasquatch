SRCDIR=$$PROJECTROOT/src
message($$SRCDIR)

HEADERS += $$PWD/mediascanner.h \
           $$PWD/mediaparser.h \
           $$PWD/playlist.h \
           $$PWD/mediamodel.h \
           $$PWD/mediaplugin.h \
    mediaplayervlc/mediaplayervlc.h

SOURCES += $$PWD/mediascanner.cpp \
           $$PWD/mediaparser.cpp \
           $$PWD/playlist.cpp \
           $$PWD/mediamodel.cpp \
    mediaplayervlc/mediaplayervlc.cpp

RESOURCES += $$PWD/media.qrc



