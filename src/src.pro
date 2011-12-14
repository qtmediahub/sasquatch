CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS += \
           corelib.pro \
           media/plugins \
           main/main.pro

qt5 {
    message("No Pushui controls available, yet")
} else {
    SUBDIRS += pushui/pushui.pro
}

