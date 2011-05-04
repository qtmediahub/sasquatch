TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          qmlmodulediscovery \
          customcursor \
          animatedtiles
#          terminalmode
#          radio

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
