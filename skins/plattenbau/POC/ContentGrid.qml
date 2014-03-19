import QtQuick 2.0

GridView {
    id: root
    width: 200
    height: 800
    cellWidth: 100
    cellHeight: 100

    model: 200
    delegate: plate

    Component {
        id: plate
        Rectangle {
            width: index == 4 ? 190 : 90
            height: index == 5 ? 0 : 90
            color: "#00ff00"
            border.color: "black"
            smooth: true
            clip: true

            transform: [
                Rotation {
                    id: plateRotation
                    angle: -90
                    axis { x: 0; y: 1; z: 0 }
                    origin.x: -200
                    origin.y: 50
                },
                Rotation {
                    id: plateFlip
                    angle: 0
                    axis { x: 1; y: 0; z: 0 }
                    origin.x: 50
                    origin.y: 50
                }
            ]

            SequentialAnimation {
                id: delegateItemAnim
                PauseAnimation { duration: Math.random()*1000 }
                NumberAnimation {
                    target: plateRotation
                    properties: "angle"
                    easing.type: Easing.OutCubic
                    from: -90
                    to: 0
                    duration: 800
                }
            }

            RotationAnimation {
                id: flipAnimation
                target: plateFlip
                easing.type: Easing.InOutCubic
                from: 0
                to: 360
                duration: 3000
                properties: "angle"
            }

            Timer {
                interval: Math.random()*50000
                repeat: true
                running: true
                onTriggered: flipAnimation.start()
            }

            SequentialAnimation {
                id: drawerAnimation
                NumberAnimation {
                    target: backgroundImage
                    easing.type: Easing.InOutCubic
                    to: 0
                    duration: 3000
                    properties: "y"
                }
                PauseAnimation { duration: 1000 }
                NumberAnimation {
                    target: backgroundImage
                    easing.type: Easing.InOutCubic
                    to: -backgroundImage.height-1
                    duration: 3000
                    properties: "y"
                }
            }

            Timer {
                interval: Math.random()*50000
                repeat: true
                running: true
                onTriggered: index % 2 ? flipAnimation.start() : drawerAnimation.start()
            }

            Rectangle {
                id: backgroundImage
                color: "#009900"
                x: 0
                y: -backgroundImage.height-1
                width: parent.width
                height: parent.height
                border.color: "black"
            }

            Component.onCompleted: delegateItemAnim.restart()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.model = 0
            root.model = 200
        }
    }
}
