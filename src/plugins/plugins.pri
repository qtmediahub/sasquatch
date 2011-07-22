include($$PROJECTROOT/src/common.pri)
include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = lib
DESTDIR = $$PROJECTROOT/hub/plugins
CONFIG += plugin

OBJECTS_DIR=.obj
MOC_DIR=.moc

INCLUDEPATH += $${PROJECTROOT}/src $${PROJECTROOT}/src/media

#Wouldn't it be nice?
#LANGUAGES = english_us.ts \
#            english_uk.ts \
#            english_bob.ts
