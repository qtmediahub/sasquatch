# This is a common file for the corelib, plugins and main
# Add with care

QT += declarative script network sql

TEMP_DIR = .tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

isEmpty(PREFIX) {
    unix:!symbian {
        maemo5 {
            PREFIX=/opt/usr
        } else {
            PREFIX=/usr/local
        }
    }
}

INSTALL_PREFIX=$$PREFIX/share/qtmediahub/
DEFINES += "QMH_INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\""
DEFINES += "QMH_DBUS_SERVICENAME=\\\"com.nokia.qtmediahub\\\""
DEFINES += "QMH_PLAYER_DBUS_SERVICENAME=\\\"com.nokia.qtmediahub.player\\\""

# default options
linux* {
    !no-gst:CONFIG += gstreamer
    !no-gl:CONFIG += glviewport
    !no-avahi: CONFIG += avahi
}

glgs {
    CONFIG += gl
    DEFINES += GLGS
}

glviewport {
    CONFIG += gl
    DEFINES += GLVIEWPORT
}

gl {
    DEFINES += GL
    QT += opengl
}

scenegraph {
    DEFINES += SCENEGRAPH
}

mac {
    QT += webkit multimedia
    CONFIG += mobility no-dbus
    MOBILITY += multimedia systeminfo
}

!contains(QT_CONFIG, dbus) {
    CONFIG += no-dbus
}

no-dbus {
    message(Disabling dbus support)
    DEFINES += NO_DBUS
}

# This is needed for Maemo5 to recognize minimization of the application window
# Thanks quit coding!
maemo5 {
    QT += dbus
}

CONFIG(debug, debug|release):CONFIG += declarative_debug

# Include JS debugger library if QMLJSDEBUGGER_PATH is set
!isEmpty(QMLJSDEBUGGER_PATH) {
    include($$QMLJSDEBUGGER_PATH/qmljsdebugger-lib.pri)
} else {
    DEFINES -= QMLJSDEBUGGER
}
