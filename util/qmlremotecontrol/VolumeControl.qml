import QtQuick 1.1

Item {
    height: width/4 + 10

    ImageButton {
        id: volumeDown
        image: "scroll-left"
        anchors.margins: 10
        anchors.right: volumeMuteToggle.left
        onClicked: rpcClient.call("qmhrpc.takeAction", 8)
        width: parent.width/4
        height: width
    }
    ImageButton {
        id: volumeMuteToggle
        image: "VolumeIcon"
        hasFocusImage: false
        anchors.margins: 10
        anchors.horizontalCenter: parent.horizontalCenter
//            onClicked: rpcClient.call("qmhrpc.takeAction", 4)
        width: parent.width/4
        height: width
    }
    ImageButton {
        id: volumeUp
        image: "scroll-right"
        anchors.margins: 10
        anchors.left: volumeMuteToggle.right
        onClicked: rpcClient.call("qmhrpc.takeAction", 7)
        width: parent.width/4
        height: width
    }
}

