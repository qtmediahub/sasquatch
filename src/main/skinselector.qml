import QtQuick 1.1

ListView {
    id: root
    focus: true

    SystemPalette { 
        id: palette
        colorGroup: SystemPalette.Active 
    }

    anchors.fill: parent
    model: skinManager.skinsModel

    delegate: Rectangle {
        id: delegate
        width: ListView.view.width
        height: text.implicitHeight
        color: ListView.isCurrentItem ? palette.highlight : "transparent"

        Text {
            id: text
            anchors.fill: parent
            text: model.modelData.name
            color: delegate.ListView.view.isCurrentItem ? palette.highlightedText : palette.text
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: delegate.ListView.view.currentIndex = index
            onDoubleClicked: window.setSkin(model.modelData.name)
        }

        Keys.onEnterPressed: window.setSkin(model.modelData.name)
        Keys.onReturnPressed: window.setSkin(model.modelData.name)
    }
}

