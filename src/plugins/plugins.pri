TEMPLATE = lib
DESTDIR = ../../../hub/plugins
CONFIG += plugin

OBJECTS_DIR=.obj
MOC_DIR=.moc

QMAKE_LFLAGS += -Wl,--rpath=.
INCLUDEPATH += ../ ../qmhplugin/

HEADERS += \
    ../submenuentry.h

SOURCES += \
    ../submenuentry.cpp

