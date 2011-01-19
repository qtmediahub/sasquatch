DEFINES += BUILDING_QMH

TEMP_DIR = tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc

CONFIG += release glviewport

linux: QMAKE_LFLAGS += -Wl,-export-dynamic
mac: QMAKE_LFLAGS += -undefined dynamic_lookup
