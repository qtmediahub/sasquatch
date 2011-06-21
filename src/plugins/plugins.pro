TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          customcursor

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
