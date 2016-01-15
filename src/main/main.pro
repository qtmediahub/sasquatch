include($$PROJECTROOT/src/core-common.pri)

TEMPLATE = app
mac:{
    DESTDIR = $$BUILDROOT/hub
} else {
    DESTDIR = $$BUILDROOT/hub/bin
}
TARGET=sasquatch
DEPENDPATH += .
INCLUDEPATH += $${PROJECTROOT}/src/

TEMP_DIR = $$PWD/.tmp
OBJECTS_DIR = $$TEMP_DIR/.obj
MOC_DIR = $$TEMP_DIR/.moc
RCC_DIR = $$TEMP_DIR/.rcc

#Including private for access to QUnifiedTimer!
QT += core-private
RESOURCES = main-qt5.qrc

# Input
SOURCES += main.cpp

RESOURCES += main.qrc

INCLUDEPATH += ../

target.path = $$PREFIX/bin
INSTALLS += target

