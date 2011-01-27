TEMPLATE = app
DESTDIR = ../../hub/
DEPENDPATH += .
INCLUDEPATH += . ../../src/3rdparty/ ../../src ../../src/qml-extensions

QT += network declarative
include(../../src/rpc/rpc.pri)

# avahi integration
INCLUDEPATH +=  ../../src/3rdparty/libqavahi/
include(../../src/3rdparty/libqavahi/libqavahi.pri)
DEFINES += HAVE_AVAHI
SOURCES += avahiservicebrowserview.cpp
HEADERS += avahiservicebrowserview.h

TEMP_DIR = $$PWD
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
UI_DIR = $$TEMP_DIR/.ui

FORMS += remotecontrol.ui

# Input
SOURCES += main.cpp remotecontrol.cpp
HEADERS += remotecontrol.h

