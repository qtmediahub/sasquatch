include($$PROJECTROOT/src/common.pri)

TEMPLATE = lib
DESTDIR = ../../../hub/plugins
CONFIG += plugin

OBJECTS_DIR=.obj
MOC_DIR=.moc

INCLUDEPATH += $${PROJECTROOT}/src/plugins/qmhplugin $${PROJECTROOT}/src
#Wouldn't it be nice?
#LANGUAGES = english_us.ts \
#            english_uk.ts \
#            english_bob.ts
