# This is a common file for the corelib, plugins and main
# Add with care

QT += declarative script network sql

TEMP_DIR = .tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

#/usr/share/apps/qtmediahub/
#/usr/share/qtmediahub/

isEmpty(PREFIX) {
    unix:!symbian {
        maemo5 {
            PREFIX=/opt/usr/
        } else {
            PREFIX=/usr/local/
        }
    }
}

INSTALL_PREFIX=$$PREFIX/share/qtmediahub

# default options
linux* {
    CONFIG += gstreamer
    !no-avahi: CONFIG += avahi
}

# default options
{
    CONFIG += glviewport
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
    CONFIG += mobility
    MOBILITY += multimedia systeminfo
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
