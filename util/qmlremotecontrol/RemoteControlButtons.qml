import QtQuick 1.1

Item {
    ImageButton {
        id: up
        image: "scroll-up"
        anchors.bottom: ok.top
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: rpcClient.call("qmhrpc.takeAction", 1)
        width: parent.width/3
        height: width
    }
    ImageButton {
        id: down
        image: "scroll-down"
        anchors.top: ok.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: rpcClient.call("qmhrpc.takeAction", 3)
        width: parent.width/3
        height: width
    }
    ImageButton {
        id: left
        image: "scroll-left"
        anchors.verticalCenter: ok.verticalCenter
        anchors.right: ok.left
        onClicked: rpcClient.call("qmhrpc.takeAction", 0)
        width: parent.width/3
        height: width
    }
    ImageButton {
        id: right
        image: "scroll-right"
        anchors.verticalCenter: ok.verticalCenter
        anchors.left: ok.right
        onClicked: rpcClient.call("qmhrpc.takeAction", 2)
        width: parent.width/3
        height: width
    }
    ImageButton {
        id: ok
        image: "ok"
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: rpcClient.call("qmhrpc.takeAction", 4)
        width: parent.width/3
        height: width
        triggerOnHold: false

        SequentialAnimation{
            id: pulseAnimation
            property int myDuration : 800
            running: qmlRemote.state == "control"
            loops: Animation.Infinite
            ParallelAnimation {
                PropertyAnimation { target: ok; property: "opacity"; to: 1.0; duration: pulseAnimation.myDuration }
                PropertyAnimation { target: ok; property: "scale"; to: 1.1; duration: pulseAnimation.myDuration}
            }
            ParallelAnimation {
                PropertyAnimation { target: ok; property: "opacity"; to: 0.5; duration: pulseAnimation.myDuration }
                PropertyAnimation { target: ok; property: "scale"; to: 1.0; duration: pulseAnimation.myDuration }
            }
        }
    }

    Button {
        id: back
        text: qsTr("Back")
        anchors.margins: 5
        anchors.top: down.bottom
        anchors.left: parent.left
        onClicked: rpcClient.call("qmhrpc.takeAction", 5)
    }

    Button {
        id: context
        text: qsTr("Context")
        anchors.margins: 5
        anchors.top: down.bottom
        anchors.right: parent.right
        onClicked: rpcClient.call("qmhrpc.takeAction", 6)
    }
}

