TEMPLATE = app
DESTDIR = $${PROJECTROOT}/hub/
DEPENDPATH += .
INCLUDEPATH += . $${PROJECTROOT}/src/3rdparty/ $${PROJECTROOT}/src $${PROJECTROOT}/src/qml-extensions

QT += network declarative
include($${PROJECTROOT}/src/rpc/rpc.pri)

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}

linux* {
    CONFIG += avahi
}

symbian {
    LIBS += -llibc -lcone -leikcore -lavkon
    TARGET.UID3 = 0xA000D7D1
    TARGET.CAPABILITY += LocalServices NetworkServices ReadUserData WriteUserData UserEnvironment
}

avahi {
    DEFINES += QMH_AVAHI
    INCLUDEPATH += . $${PROJECTROOT}/src/3rdparty/libqavahi
    include($${PROJECTROOT}/src/3rdparty/libqavahi/libqavahi.pri)
} else {
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

RESOURCES += qmlremotecontrol.qrc

OTHER_FILES += \
    qmlremotecontrol.qml \
    Button.qml \
    ImageButton.qml \
    ControlView.qml \
    BusyView.qml \
    BusyIndicator.qml \
    TargetsView.qml \
    AddTargetDialog.qml

