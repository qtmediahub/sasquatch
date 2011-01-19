mac {
    QMAKE_LFLAGS += -undefined dynamic_lookup
} else {
    QMAKE_LFLAGS += -Wl,-export-dynamic
}
