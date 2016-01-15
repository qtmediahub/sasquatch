include(src/common.pri)

CONFIG += ordered

TEMPLATE = subdirs

# add additional folders for keymaps, qml imports and resources to install step
folder_01.source=share/sasquatch/keymaps
folder_02.source=share/sasquatch/imports
folder_03.source=share/sasquatch/resources
DEPLOYMENTFOLDERS=folder_01 folder_02 folder_03

installPrefix = $$PREFIX/share/sasquatch/
for(deploymentfolder, DEPLOYMENTFOLDERS) {
    item = item$${deploymentfolder}
    itemfiles = $${item}.files
    $$itemfiles = $$eval($${deploymentfolder}.source)
    itempath = $${item}.path
    $$itempath = $${installPrefix}/$$eval($${deploymentfolder}.target)
    export($$itemfiles)
    export($$itempath)
    INSTALLS += $$item
}

#    include(generated-deployment.pri)
#    qtcAddDeployment()

SUBDIRS += src

!no-dbus {
  SUBDIRS += mediaplayer
} else {
  message("dbus mediaplayer helper app not being built")
}

mac {
    message(You are compiling on mac: displaying mac specific notes)
    system(cat ./mac-platform-notes)
}

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog
