TEMPLATE = app
DESTDIR = ../../hub/
DEPENDPATH += .
INCLUDEPATH += . ../../src/3rdparty/ ../../src

QT += network
include(../../src/rpc/rpc.pri)

TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

FORMS += remotecontrol.ui

# Input
SOURCES += main.cpp remotecontrol.cpp
HEADERS += remotecontrol.h

