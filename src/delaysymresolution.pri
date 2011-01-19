linux: QMAKE_LFLAGS += -Wl,-export-dynamic
mac: QMAKE_LFLAGS += -undefined dynamic_lookup
