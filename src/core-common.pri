include($$PROJECTROOT/src/common.pri)

CORE_LIB_DIR = $$PROJECTROOT/lib/

QMAKE_LIBDIR += $${CORE_LIB_DIR}

LIBS += -L$${BUILDROOT}/lib/ -lqmhcore
QMAKE_LFLAGS += -Wl,-rpath,$${PREFIX}/lib/ -Wl,-rpath,$${BUILDROOT}/lib/

mac {
    QMAKE_LFLAGS += -mmacosx-version-min=10.5
}
