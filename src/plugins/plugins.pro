TEMPLATE = subdirs

SUBDIRS = music \
          video \
          radio \
          picture \
          qmlmodulediscovery \
          customcursor \
          animatedtiles
#          terminalmode

dbus {
    SUBDIRS += appstore
}
