CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS += \
           corelib.pro \
           media/plugins \
           main/main.pro

media-backend {
    SUBDIRS += mediabackend
}
