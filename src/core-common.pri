include($$PROJECTROOT/src/common.pri)

CORE_LIB_DIR = $$PROJECTROOT/hub/lib/

QMAKE_LIBDIR += $${CORE_LIB_DIR}

LIBS += -lqmhcore
QMAKE_LFLAGS += -Wl,-rpath,$${INSTALL_PREFIX}/lib/ -Wl,-rpath,$${CORE_LIB_DIR}

mac {
QMAKE_LFLAGS += -mmacosx-version-min=10.5
}
