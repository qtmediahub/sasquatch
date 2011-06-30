SRCDIR=$$PROJECTROOT/src
message($$SRCDIR)

HEADERS += $$PWD/mediascanner.h \
           $$PWD/mediaparser.h \
           $$PWD/media.h \
           $$PWD/playlist.h \
           $$PWD/mediamodel.h

SOURCES += $$PWD/mediascanner.cpp \
           $$PWD/mediaparser.cpp \
           $$PWD/media.cpp \
           $$PWD/playlist.cpp \
           $$PWD/mediamodel.cpp

RESOURCES += $$PWD/media.qrc

