include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = lib
DESTDIR = $$PROJECTROOT/lib/qtmediahub/
CONFIG += plugin
QT += sql

OBJECTS_DIR=.obj
MOC_DIR=.moc

INCLUDEPATH += $${PROJECTROOT}/src $${PROJECTROOT}/src/media

#Wouldn't it be nice?
#LANGUAGES = english_us.ts \
#            english_uk.ts \
#            english_bob.ts

target.path = $${PREFIX}/lib/qtmediahub/
INSTALLS += target

TARGET = $${TARGET}parser
