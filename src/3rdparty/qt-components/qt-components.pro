TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src 
#          tests \
#          examples

isEmpty(Q_COMPONENTS_SOURCE_TREE)|isEmpty(Q_COMPONENTS_BUILD_TREE) {
    message(This should not be happening, .qmake.cache should exist in the top level directory)
    unix:system(cat $PROJECTROOT/.qmake.cache)
    message(If you see nothing above please check that this file is there)
}
