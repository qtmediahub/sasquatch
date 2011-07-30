QMAKE_CACHE=$${OUT_PWD}/.qmake.cache
BASE_NAME=$$PWD

# Detect stale .qmake.cache
exists($$QMAKE_CACHE) {
    !equals(PROJECTROOT, $$BASE_NAME): system(rm $$QMAKE_CACHE)
}

isEmpty(PROJECTROOT) {
    system(echo "PROJECTROOT=$$BASE_NAME" > $$QMAKE_CACHE)
    system(echo "CONFIG-=qt_framework" >> $$QMAKE_CACHE)
    error(Cache created; please rerun qmake)
}
