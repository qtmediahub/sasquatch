TEMPLATE = subdirs

QMAKE_CACHE=$${OUT_PWD}/.qmake.cache

!exists($$QMAKE_CACHE) {
    message(creating .qmake.cache)
    message(Dont check it in and be sure to flush it if you relocate your src tree)
#gonna want a Windows friendly version of this as well
    system(echo "PROJECTROOT=$$PWD" > $$QMAKE_CACHE)
    system(echo "Q_COMPONENTS_SOURCE_TREE=$$PWD/src/3rdparty/qt-components" >> $$QMAKE_CACHE)
    system(echo "Q_COMPONENTS_BUILD_TREE=$$PWD/hub/resources" >> $$QMAKE_CACHE)
    system(echo "CONFIG+=mx" >> $$QMAKE_CACHE)
    system(echo "CONFIG-=qt_framework" >> $$QMAKE_CACHE)
}

SUBDIRS = src \
          src/plugins \
          src/3rdparty/qt-components/qt-components.pro \
          util/remotecontrol \
          util/qmlremotecontrol
