TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src 
#          tests \
#          examples

isEmpty(Q_COMPONENTS_SOURCE_TREE)|isEmpty(Q_COMPONENTS_BUILD_TREE) {
    message(Causality problem: .qmake.cache needs to exist in your root directory before you hit this point)
    error(Please re-run: qmake -r .......)
}
