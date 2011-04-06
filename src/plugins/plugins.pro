TEMPLATE = subdirs

SUBDIRS = music \
          video \
          radio \
          picture \
          qmlmodulediscovery \
          customcursor \
          animatedtiles
#          terminalmode

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
