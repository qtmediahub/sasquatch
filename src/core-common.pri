include($$PROJECTROOT/src/common.pri)

CORE_LIB_DIR = $$PROJECTROOT/hub/lib/

QMAKE_LIBDIR += $${CORE_LIB_DIR}

LIBS += -L$${BUILDROOT}/hub/lib/ -lqmhcore
QMAKE_LFLAGS += -Wl,-rpath,$${PREFIX}/lib/ -Wl,-rpath,$${BUILDROOT}/hub/lib/

mac {
    QMAKE_LFLAGS += -mmacosx-version-min=10.5
}
