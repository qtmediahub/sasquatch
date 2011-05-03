import QtQuick 1.1

Column {
    property variant target
    width: target.width; height: target.height

    default property alias content: container.children
    property alias gripText: grip.text

    MouseArea {
        id: container
        height: parent.height - grip.height
        width: parent.width
    }

    Grip {
        id: grip
    }
}
