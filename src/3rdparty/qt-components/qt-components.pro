TEMPLATE = subdirs
CONFIG += ordered
#          tests \
#          examples

isEmpty(Q_COMPONENTS_SOURCE_TREE)|isEmpty(Q_COMPONENTS_BUILD_TREE) {
    message(Causality problem: .qmake.cache needs to exist in your root directory before you hit this point)
    message(Wont built Qt components)
} else {
    SUBDIRS = src
}
