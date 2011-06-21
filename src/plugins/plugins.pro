TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          customcursor
#          terminalmode
#          radio

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
