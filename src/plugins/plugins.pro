TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture \
          radio \
          customcursor

contains(QT_CONFIG, dbus) {
    SUBDIRS += appstore
}
