TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          customcursor
#          radio

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
