import QtQuick 1.1

BorderImage {
    id: trackpad
    source: "media/trackpad.png"
    smooth: true
    border { top: 20; left: 20; right: 20; bottom: 20 }

    MouseArea {
        property int lastX: -1
        property int lastY: -1
        property int startX: -1
        property int startY: -1
        property bool inited: false
        property bool ignoreClick: false

        function manhattanLength(x, y) { return Math.sqrt(x*x + y*y) }

        anchors.fill: parent
        onPressed: { startX = startY = -1; ignoreClick = false; rpcClient.callBool("trackpad.setEnabled", true) }
        onReleased: { inited = false; }
        onClicked: if (!ignoreClick) { rpcClient.callVoid("trackpad.click") }

        onPositionChanged: {
            if (inited) {
                rpcClient.call2("trackpad.moveBy", mouse.x - lastX, mouse.y - lastY)
            }

            if (startX == -1) startX = mouse.x
            if (startY == -1) startY = mouse.y

            if (manhattanLength(startX - mouse.x, startY - mouse.y) > 16)
                ignoreClick = true

            lastX = mouse.x
            lastY = mouse.y
            inited = true
        }
    }
}

