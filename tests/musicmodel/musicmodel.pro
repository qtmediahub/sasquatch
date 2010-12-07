TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += . ../../src/plugins/music/ ../../src/3rdparty/
QT += declarative
CONFIG += debug


include(../../src/3rdparty/taglib/taglib.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc

# Input
HEADERS += ../../src/plugins/music/musicmodel.h \
           ../../src/plugins/mediamodel.h

SOURCES += main.cpp \
           ../../src/plugins/music/musicmodel.cpp \
           ../../src/plugins/mediamodel.cpp

