import QtQuick 1.1

BorderImage {
    id: trackpad
    source: "media/trackpad.png"
    smooth: true
    border { top: 20; left: 20; right: 20; bottom: 20 }

    MouseArea {
        property int lastX: -1
        property int lastY: -1
        property bool inited: false
        anchors.fill: parent
        onPressed: rpcClient.callBool("trackpad.setEnabled", true)
        onReleased: inited = false
        onPositionChanged: {
            if (inited) {
                rpcClient.call2("trackpad.moveBy", mouse.x - lastX, mouse.y - lastY)
            }
            lastX = mouse.x
            lastY = mouse.y
            inited = true
        }
    }
}

