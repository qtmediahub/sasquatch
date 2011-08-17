import QtQuick 1.0

QtObject {
    function createQmlObjectFromFile(file, properties, parent) {
        var qmlComponent = Qt.createComponent(file)
        if (qmlComponent.status == Component.Ready) {
            return qmlComponent.createObject(parent ? parent : this, properties ? properties : {})
        }
        console.log(qmlComponent.errorString())
        return null
    }

    function createBinding(target, targetProperty, value, owner) {
        Qt.createQmlObject('import QtQuick 1.0; Binding { target: ' + target + '; property: "' + targetProperty + '"; value: ' + value +' }', owner)
    }
}
