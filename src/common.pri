# This is a common file for the corelib, plugins and main
# Add with care

isEmpty(PROJECTROOT) {
    message("Building without a rational PROJECTROOT is undefined")
    error("Did you run configure?")
}

qt5 {
    DEFINES += QT5
    QT += v8 network sql quick widgets declarative qml#QDirModel from widgets!
} else {
    QT += declarative script network sql
}

TEMP_DIR = .tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

isEmpty(PREFIX) {
    #Use our fuzzy heuristic logic for prefix setting
    mac {
        PREFIX=$$PROJECTROOT
    }
    maemo5 {
        PREFIX=/opt/usr/
    }
    harmattan {
        PREFIX=/opt/qtmediahub/
    }
    isEmpty(PREFIX) {
        PREFIX=/usr/local/
    }
}

DEFINES += "QMH_PREFIX=\\\"$$PREFIX\\\""
DEFINES += "QMH_PROJECTROOT=\\\"$$PROJECTROOT\\\""
DEFINES += "QMH_DBUS_SERVICENAME=\\\"com.nokia.qtmediahub\\\""
DEFINES += "QMH_PLAYER_DBUS_SERVICENAME=\\\"com.nokia.qtmediahub.player\\\""

PKG_CONFIG_PATH=$$(PKG_CONFIG_PATH)
mac:isEmpty(PKG_CONFIG_PATH) {
    message(pkg-config usage on mac requires:)
    message(export PKG_CONFIG_PATH=/opt/local/lib/pkgconfig)
}

# default options
linux* {
    !no-gl:CONFIG += glviewport
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

mac {
    # QT += webkit multimedia
    CONFIG += no-dbus
    #CONFIG += mobility
    #MOBILITY += multimedia systeminfo
}

!contains(QT_CONFIG, dbus) {
    #This catches dbus free Qt builds
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

CONFIG(debug, debug|release):!qt5:CONFIG += declarative_debug

# Include JS debugger library if QMLJSDEBUGGER_PATH is set
!isEmpty(QMLJSDEBUGGER_PATH) {
    include($$QMLJSDEBUGGER_PATH/qmljsdebugger-lib.pri)
} else {
    DEFINES -= QMLJSDEBUGGER
}
