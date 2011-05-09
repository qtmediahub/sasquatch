TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          customcursor \
          animatedtiles
#          terminalmode
#          radio

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
