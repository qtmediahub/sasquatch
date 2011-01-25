TEMPLATE = app
DESTDIR = ../../hub/
DEPENDPATH += .
INCLUDEPATH += . ../../src/3rdparty/ ../../src

QT += network
include(../../src/rpc/rpc.pri)

# avahi integration
INCLUDEPATH +=  ../../src/3rdparty/libqavahi/
include(../../src/3rdparty/libqavahi/libqavahi.pri)
DEFINES += HAVE_AVAHI
SOURCES += avahiservicebrowserview.cpp
HEADERS += avahiservicebrowserview.h

TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

FORMS += remotecontrol.ui

# Input
SOURCES += main.cpp remotecontrol.cpp
HEADERS += remotecontrol.h

