/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

import QtQuick 1.0

Item {
    id: root

    clip: true

    property alias title : controlTitle.text

    Text {
        id: controlTitle
        color: "lightgray"
        font.weight: Font.Light
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    VolumeControl {
        id: volumeControl
        anchors.top: controlTitle.bottom
        width: parent.width
    }

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


    Button {
        id: targets
        text: qsTr("Targets")
        anchors.margins: 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: rpcClient.disconnectFromHost()
    }
}
