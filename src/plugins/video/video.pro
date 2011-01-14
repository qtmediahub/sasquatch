######################################################################
# Automatically generated by qmake (2.01a) Mon Nov 15 23:01:55 2010
######################################################################

include(../plugins.pri)

QT += declarative

gstreamer {
    message(Using gstreamer for thumbnails)
    DEFINES += THUMBNAIL_GSTREAMER

    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-0.10
}

HEADERS += \
        videoplugin.h \
        ../mediamodel.h \
        ../mediainfo.h \
        videomodel.h

SOURCES += \
        videoplugin.cpp \
        videomodel.cpp
