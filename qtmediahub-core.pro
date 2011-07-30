include(src/common.pri)
include(.cache-cow.pri)

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

linux* {
    # If you change the defaults here, fix src/common.pri
    message(Linux specific options: ('*' indicates that option is enabled by default))
    message(Use CONFIG+=<option> to enable and CONFIG-=<option> to disable)
    message(gstreamer* - Thumbnail videos using gstreamer)
    message(avahi* - Enabled Avahi support)
    message(glviewport* - Render using a GL Viewport)
    message(glgs - Render using GL graphics system (Avoid like the plague!))
}

PKG_CONFIG_PATH=$$(PKG_CONFIG_PATH)
isEmpty(PKG_CONFIG_PATH) {
    message(pkg-config usage on mac requires:)
    message(export PKG_CONFIG_PATH=/opt/local/lib/pkgconfig)
}

# due to wrong deploysteps in creator for symbian,
# comment SUBDIRS below out
SUBDIRS += \
           src
