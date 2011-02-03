DEFINES += BUILDING_QMH

TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

linux* {
    message(default to using gstreamer lib to directly generate thumbnail on linux)
    CONFIG += gstreamer avahi
}

CONFIG += release glviewport
