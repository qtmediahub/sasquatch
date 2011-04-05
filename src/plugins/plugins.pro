TEMPLATE = subdirs

SUBDIRS = qmhplugin\
          music \
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
