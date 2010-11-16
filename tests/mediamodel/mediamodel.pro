TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += . ../../src ../../src/3rdparty/

include(../../src/3rdparty/taglib/taglib.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc

# Input
HEADERS += ../../src/mediamodel.h

SOURCES += main.cpp \
           ../../src/mediamodel.cpp

