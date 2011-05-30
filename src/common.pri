# This is a common file for the corelib, plugins and main
# Add with care

QT += declarative script network sql

TEMP_DIR = .tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

# default options
linux* {
    CONFIG += gstreamer avahi glviewport
}

# default options
mac* {
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

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/bin
    } else {
        target.path = /usr/local/bin
    }
    INSTALLS += target
}

CONFIG += release
