include(.cache-cow.pri)

!isEmpty(PROJECTROOT) {

    include(src/common.pri)

    CONFIG += ordered

    TEMPLATE = subdirs

    # add additional folders for keymaps, qml imports and resources to install step
    folder_01.source =  hub/share/qtmediahub/keymaps \
                        hub/share/qtmediahub/imports \
                        hub/share/qtmediahub/resources
    DEPLOYMENTFOLDERS = folder_01

    for(deploymentfolder, DEPLOYMENTFOLDERS) {
        item = item$${deploymentfolder}
        itemfiles = $${item}.files
        $$itemfiles = $$eval($${deploymentfolder}.source)
        itempath = $${item}.path
        $$itempath = $${PREFIX}/share/qtmediahub/$$eval($${deploymentfolder}.target)
        export($$itemfiles)
        export($$itempath)
        INSTALLS += $$item
    }

    message()
    message("If you want to use QtMediaHub without make install step (in-source), use PREFIX=$PWD/hub as qmake argument")
    message("current PREFIX is "$$PREFIX)
    message()

    # due to wrong deploysteps in creator for symbian,
    # comment SUBDIRS below out
    SUBDIRS += \
               src

    OTHER_FILES += \
        qtc_packaging/debian_harmattan/rules \
        qtc_packaging/debian_harmattan/README \
        qtc_packaging/debian_harmattan/copyright \
        qtc_packaging/debian_harmattan/control \
        qtc_packaging/debian_harmattan/compat \
        qtc_packaging/debian_harmattan/changelog
}
