DESTDIR = ../../../hub
TEMPLATE = lib
TARGET = qmhplugin
DEPENDPATH += .
INCLUDEPATH += .
TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

HEADERS += qmhplugin.h

SOURCES += qmhplugin.cpp

