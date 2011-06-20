include(src/common.pri)

CONFIG += ordered

TEMPLATE = subdirs

QMAKE_CACHE=$${OUT_PWD}/.qmake.cache

# Detect stale .qmake.cache
exists($$QMAKE_CACHE) {
    include($$QMAKE_CACHE)
    !equals(PROJECTROOT, $$PWD): system(rm $$QMAKE_CACHE)
}

!exists($$QMAKE_CACHE) {
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
    message(glviewport* - Render using a GL Viewport)
    message(glgs - Render using GL graphics system (Avoid like the plague!))
}

PKG_CONFIG_PATH=$$(PKG_CONFIG_PATH)
isEmpty(PKG_CONFIG_PATH) {
    message(pkg-config usage on mac requires:)
    message(export PKG_CONFIG_PATH=/opt/local/lib/pkgconfig)
}

# due to wrong deploysteps in creator for symbian,
# comment SUBDIRS below out
SUBDIRS += \
           src

glviewport {
    SUBDIRS += src/3rdparty/qml1-shadersplugin/src/qml1-shadersplugin.pro
}
