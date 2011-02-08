TEMPLATE = app
DESTDIR = ../../hub/
DEPENDPATH += .
INCLUDEPATH += . ../../src/3rdparty/ ../../src ../../src/qml-extensions

QT += network declarative
include(../../src/rpc/rpc.pri)

!symbian {
    !no-avahi {
        # avahi integration
        INCLUDEPATH +=  ../../src/3rdparty/libqavahi/
        include(../../src/3rdparty/libqavahi/libqavahi.pri)
        SOURCES +=
        HEADERS +=
    }
} else {
    FORMS +=
    LIBS += -llibc
    TARGET.CAPABILITY += NetworkServices
}

no-avahi {
    DEFINES += QMH_NO_AVAHI
    SOURCES += staticservicebrowsermodel.cpp
    HEADERS += staticservicebrowsermodel.h
}

TEMP_DIR = $$PWD
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
UI_DIR = $$TEMP_DIR/.ui

FORMS +=

# Input
SOURCES += main.cpp \
    mainwindow.cpp
HEADERS += \
    mainwindow.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}

RESOURCES += qmlremotecontrol.qrc

OTHER_FILES += \
    qmlremotecontrol.qml \
    Button.qml \
    ImageButton.qml

