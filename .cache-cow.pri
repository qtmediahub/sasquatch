QMAKE_CACHE=$${OUT_PWD}/.qmake.cache
BASE_NAME=$$PWD

# Detect stale .qmake.cache
exists($$QMAKE_CACHE) {
    !equals(PROJECTROOT, $$BASE_NAME): system(rm $$QMAKE_CACHE)
}

isEmpty(PROJECTROOT) {
    system(echo "PROJECTROOT=$$BASE_NAME" > $$QMAKE_CACHE)
    system(echo "CONFIG-=qt_framework" >> $$QMAKE_CACHE)
    error(Cache created; please rerun qmake)
}

linux* {
    # If you change the defaults here, fix src/common.pri
    message(We set these variables in the toplevel .qmake.cache file)
    message(that qmake ascends to in perpetuity)
    message()
    message(Linux specific options: ('*' indicates that option is enabled by default))
    message(Use CONFIG+=<option> to enable and CONFIG-=<option> to disable)
    message(avahi* - Enabled Avahi support)
    message(glviewport* - Render using a GL Viewport)
    message(glgs - Render using GL graphics system (Avoid like the plague!))
}


