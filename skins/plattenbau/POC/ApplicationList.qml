import QtQuick 2.0
import MediaModel 1.0

ListView {
    id: root
    width: 400
    height: 800
    spacing: 10

    model: musicModel

    MediaModel {
        id: musicModel
        mediaType: "music"
        structure: "artist,album|title"
    }

    delegate:
        Rectangle {
        id: plate
        width: ListView.view.width
        height: 50
        color: "#00ff00"
        border.color: "black"
        smooth: true
        clip: true

        transform: Rotation {
            id: plateRotation
            angle: 270
            axis { x: 0; y: 1; z: 0 }
            origin.x: 200 + parent.width
            origin.y: 50
        }

        Text {
            anchors.centerIn: parent
            text: model.dotdot ? ".." : (musicModel.part == "artist,album" ? model.artist + " ............" + model.album : model.title)
        }

        MouseArea {
            anchors.fill: parent
            onClicked: musicModel.enter(index)
        }

        SequentialAnimation {
            id: delegateItemAnim
            PauseAnimation { duration: Math.random()*1000 }
            NumberAnimation {
                target: plateRotation
                properties: "angle"
                easing.type: Easing.OutCubic
                from: 270
                to: 0
                duration: 800
            }
        }

        Component.onCompleted: delegateItemAnim.restart()
    }
}
