QT += network
LIBS += -lavahi-common -lavahi-client -lavahi-qt4

# Input
SOURCES += $$PWD/qavahiservicepublisher.cpp $$PWD/qavahiservicebrowsermodel.cpp
HEADERS += $$PWD/qavahiservicepublisher.h $$PWD/qavahiservicebrowsermodel.h

