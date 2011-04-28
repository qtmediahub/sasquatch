include(../../3rdparty/libexif-0.6.19/libexif/libexif.pri)
include(../3rdparty/taglib/taglib.pri)

INCLUDEPATH += ../3rdparty/taglib/ ../3rdparty/ 

HEADERS += $$PWD/mediainfo.h \
           $$PWD/mediamodel.h \
           $$PWD/tagreader.h \
           $$PWD/exifreader.h

SOURCES += $$PWD/mediainfo.cpp \
           $$PWD/mediamodel.cpp \
           $$PWD/tagreader.cpp \
           $$PWD/exifreader.cpp 

