include(src/common.pri)

CONFIG += ordered

TEMPLATE = subdirs

QMAKE_CACHE=$${OUT_PWD}/.qmake.cache

!exists($$QMAKE_CACHE) {
    message(creating .qmake.cache)
    message(Dont check it in and be sure to flush it if you relocate your src tree)
    # gonna want a Windows friendly version of this as well
    system(echo "PROJECTROOT=$$PWD" > $$QMAKE_CACHE)
    system(echo "Q_COMPONENTS_SOURCE_TREE=$$PWD/src/3rdparty/qt-components" >> $$QMAKE_CACHE)
    system(echo "Q_COMPONENTS_BUILD_TREE=$$PWD/hub/resources" >> $$QMAKE_CACHE)
    system(echo "CONFIG+=mx" >> $$QMAKE_CACHE)
    system(echo "CONFIG-=qt_framework" >> $$QMAKE_CACHE)
}

linux* {
    # If you change the defaults here, fix src/common.pri
    message(Linux specific options: ('*' indicates that option is enabled by default))
    message(Use CONFIG+=<option> to enable and CONFIG-=<option> to disable)
    message(gstreamer* - Thumbnail videos using gstreamer)
    message(avahi* - Enabled Avahi support)
    message(glviewport* - Rener using a GL Viewport)
    message(glgs - Render using GL graphics system)
}

# due to wrong deploysteps in creator for symbian,
# comment SUBDIRS below out
SUBDIRS += src
