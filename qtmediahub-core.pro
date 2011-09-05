include(.cache-cow.pri)

!isEmpty(PROJECTROOT) {

    include(src/common.pri)

    CONFIG += ordered

    TEMPLATE = subdirs

    folder_01.source = hub/devices hub/imports hub/resources
    DEPLOYMENTFOLDERS = folder_01

    #for(deploymentfolder, DEPLOYMENTFOLDERS) {
    #    item = item$${deploymentfolder}
    #    itemsources = $${item}.sources
    #    $$itemsources = $$eval($${deploymentfolder}.source)
    #    itempath = $${item}.path
    #    $$itempath= $$eval($${deploymentfolder}.target)
    #    export($$itemsources)
    #    export($$itempath)
    #    DEPLOYMENT += $$item
    #}

    CONFIG(android) : installPrefix = /assets$${INSTALL_PREFIX}
               else : installPrefix = $${INSTALL_PREFIX}

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
