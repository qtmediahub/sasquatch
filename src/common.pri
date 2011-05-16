DEFINES += BUILDING_QMH

QT += declarative sql

TEMP_DIR = .tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

# default options
linux* {
    CONFIG += gstreamer avahi
}

CONFIG += release
