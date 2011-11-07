include(.cache-cow.pri)

#Used for mac app-bundle deployment
DESTDIR = $$PROJECTROOT/qtmediahub-core/bin
TARGET=qtmediahub

!isEmpty(PROJECTROOT) {

    include(src/common.pri)

    CONFIG += ordered

    TEMPLATE = subdirs

    # add additional folders for keymaps, qml imports and resources to install step
    folder_01.source=share/qtmediahub/keymaps
    folder_02.source=share/qtmediahub/imports
    folder_03.source=share/qtmediahub/resources
    DEPLOYMENTFOLDERS=folder_01 folder_02 folder_03

    include(generated-deployment.pri)
    qtcAddDeployment()

    message()
    message("If you want to set an explicit deployment (make install) PREFIX please add it to .qmake.cache or use PREFIX=$PWD as recursive qmake argument")
    message("QtMediaHub is built to run in-source (without make install) by default")
    message("current PREFIX is "$$PREFIX)
    message()

    SUBDIRS += src

    OTHER_FILES += \
        qtc_packaging/debian_harmattan/rules \
        qtc_packaging/debian_harmattan/README \
        qtc_packaging/debian_harmattan/copyright \
        qtc_packaging/debian_harmattan/control \
        qtc_packaging/debian_harmattan/compat \
        qtc_packaging/debian_harmattan/changelog
}
