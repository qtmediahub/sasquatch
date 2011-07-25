TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          radio

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
