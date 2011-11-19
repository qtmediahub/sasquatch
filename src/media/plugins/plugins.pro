TEMPLATE = subdirs

SUBDIRS = music \
          video \
          picture

radio-plugin {
    SUBDIRS += radio
}

snes-plugin {
    SUBDIRS += snes
}

