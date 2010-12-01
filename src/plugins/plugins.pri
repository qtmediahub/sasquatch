TEMPLATE = lib
DESTDIR = ../../../hub/plugins
CONFIG += plugin

OBJECTS_DIR=.obj
MOC_DIR=.moc

INCLUDEPATH += ../

HEADERS += \
    ../submenuentry.h

SOURCES += \
    ../submenuentry.cpp

#Wouldn't it be nice?
#LANGUAGES = english_us.ts \
#            english_uk.ts \
#            english_bob.ts
