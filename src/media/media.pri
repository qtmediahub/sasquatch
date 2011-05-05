SRCDIR=$$PROJECTROOT/src
message($$SRCDIR)

include($$SRCDIR/3rdparty/libexif-0.6.19/libexif/libexif.pri)
include($$SRCDIR/3rdparty/taglib/taglib.pri)

INCLUDEPATH += $$SRCDIR/3rdparty/taglib/ $$SRCDIR/3rdparty/ $$SRCDIR/3rdparty/libexif-0.6.19

HEADERS += $$PWD/tagreader.h \
           $$PWD/exifreader.h \
           $$PWD/mediascanner.h \
           $$PWD/mediaparser.h \
           $$PWD/media.h \
           $$PWD/playlist.h

SOURCES += $$PWD/tagreader.cpp \
           $$PWD/exifreader.cpp \
           $$PWD/mediascanner.cpp \
           $$PWD/mediaparser.cpp \
           $$PWD/media.cpp \
           $$PWD/playlist.cpp

RESOURCES += $$PWD/media.qrc

