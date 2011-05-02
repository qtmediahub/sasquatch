SRCDIR=$$PROJECTROOT/src
message($$SRCDIR)

include($$SRCDIR/3rdparty/libexif-0.6.19/libexif/libexif.pri)
include($$SRCDIR/3rdparty/taglib/taglib.pri)

INCLUDEPATH += $$SRCDIR/3rdparty/taglib/ $$SRCDIR/3rdparty/ $$SRCDIR/3rdparty/libexif-0.6.19

HEADERS += $$PWD/mediainfo.h \
           $$PWD/mediamodel.h \
           $$PWD/tagreader.h \
           $$PWD/exifreader.h

SOURCES += $$PWD/mediainfo.cpp \
           $$PWD/mediamodel.cpp \
           $$PWD/tagreader.cpp \
           $$PWD/exifreader.cpp