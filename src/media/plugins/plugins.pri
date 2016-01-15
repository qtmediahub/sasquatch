include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = lib
DESTDIR = $$BUILDROOT/hub/lib/sasquatch/
CONFIG += plugin
QT += sql

OBJECTS_DIR=.obj
MOC_DIR=.moc

INCLUDEPATH += $${PROJECTROOT}/src $${PROJECTROOT}/src/media

#Wouldn't it be nice?
#LANGUAGES = english_us.ts \
#            english_uk.ts \
#            english_bob.ts

target.path = $${PREFIX}/hub/lib/sasquatch/
INSTALLS += target

TARGET = $${TARGET}parser
