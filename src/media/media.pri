SRCDIR=$$PROJECTROOT/src
message($$SRCDIR)

INCLUDEPATH += $$SRCDIR/3rdparty/

HEADERS += $$PWD/mediascanner.h \
           $$PWD/mediaparser.h \
           $$PWD/media.h \
           $$PWD/playlist.h

SOURCES += $$PWD/mediascanner.cpp \
           $$PWD/mediaparser.cpp \
           $$PWD/media.cpp \
           $$PWD/playlist.cpp

RESOURCES += $$PWD/media.qrc

