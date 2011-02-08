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

import Qt 4.7
import RpcConnection 1.0

Rectangle {
    id: root
    height: 640
    width: 360
    color: "black"

    state: "targets"

    Image {
        id: background
        anchors.fill: parent
        source: "qrc:/media/media-overlay.png"
    }

    RpcConnection {
        id: rpcClient
        property string source
        property int position

        onClientConnected: {
            root.state = "control"
        }

        onClientDisconnected: {
            root.state = "targets"
        }

        function send(ip, port, src, pos) {
            source = src
            position = pos
            connectToHost(ip, port);
        }
    }

    states: [
        State {
            name: "targets"
            PropertyChanges {
                target: targetsView.anchors
                leftMargin: 0
            }
            PropertyChanges {
                target: controlView.anchors
                leftMargin: -controlView.width
            }
        },
        State {
            name: "control"
            PropertyChanges {
                target: targetsView.anchors
                leftMargin: -targetsView.width
            }
            PropertyChanges {
                target: controlView.anchors
                leftMargin: 0
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation { properties: "leftMargin"; }
        }
    ]

    Item {
        id: targetsView

        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: parent.height
        clip: true

        Text {
            id: targetsTitle
            text: qsTr("Select Target")
            color: "lightgray"
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
            font.weight: Font.Light
        }

        ListView {
            id: targetsList
            model: targetsModel

            anchors.top: targetsTitle.bottom
            anchors.topMargin: 20
            anchors.bottom: targetsView.bottom
            anchors.bottomMargin: 20
            width: parent.width
            clip: true

            delegate: Item {
                width: ListView.view.width
                height: sourceText.height + 8
                Image {
                    id: backgroundImage
                    anchors.fill: parent;
                    source: "qrc:/media/" + (ListView.isCurrentItem ? "MenuItemFO.png" : "MenuItemNF.png");
                }
                Text {
                    id: sourceText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    z: 1 // ensure it is above the background
                    text: model.display
                    font.weight: Font.Light
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent;
                    hoverEnabled: true
                    onEntered: ListView.view.currentIndex = index
                    onClicked: {
                        controlTitle.text = "Connected to "+model.display
                        rpcClient.connectToHost(model.address, model.port)
                    }
                }
            }
        }

        Button {
            id: exitButton
            text: "Exit"
            anchors.margins: 10
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: Qt.quit();
        }
    }

    Item {
        id: controlView

        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: parent.height
        clip: true

        property int buttonWidth : 70

        Text {
            id: controlTitle
            color: "lightgray"
            font.weight: Font.Light
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
        }

        ImageButton {
            id: volumeDown
            image: "scroll-left"
            anchors.margins: 10
            anchors.top: controlTitle.bottom
            anchors.right: volumeMuteToggle.left
            onClicked: rpcClient.call("qmhrpc.takeAction", 7)
        }
        ImageButton {
            id: volumeMuteToggle
            image: "VolumeIcon"
            hasFocusImage: false
            anchors.margins: 10
            anchors.top: controlTitle.bottom
            anchors.horizontalCenter: parent.horizontalCenter
//            onClicked: rpcClient.call("qmhrpc.takeAction", 4)
        }
        ImageButton {
            id: volumeUp
            image: "scroll-right"
            anchors.margins: 10
            anchors.top: controlTitle.bottom
            anchors.left: volumeMuteToggle.right
            onClicked: rpcClient.call("qmhrpc.takeAction", 8)
        }


        ImageButton {
            id: up
            image: "scroll-up"
            anchors.bottom: ok.top
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: rpcClient.call("qmhrpc.takeAction", 1)
            width: parent.width/4
            height: width
        }
        ImageButton {
            id: down
            image: "scroll-down"
            anchors.top: ok.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: rpcClient.call("qmhrpc.takeAction", 3)
            width: parent.width/4
            height: width
        }
        ImageButton {
            id: left
            image: "scroll-left"
            anchors.verticalCenter: ok.verticalCenter
            anchors.right: ok.left
            onClicked: rpcClient.call("qmhrpc.takeAction", 0)
            width: parent.width/4
            height: width
        }
        ImageButton {
            id: right
            image: "scroll-right"
            anchors.verticalCenter: ok.verticalCenter
            anchors.left: ok.right
            onClicked: rpcClient.call("qmhrpc.takeAction", 2)
            width: parent.width/4
            height: width
        }
        ImageButton {
            id: ok
            image: "ok"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: rpcClient.call("qmhrpc.takeAction", 4)
            width: parent.width/4
            height: width

            SequentialAnimation{
                id: pulseAnimation
                property int myDuration : 800
                running: root.state == "control"
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
            id: menu
            text: "Menu"
            anchors.margins: 10
            anchors.top: down.bottom
            anchors.left: parent.left
            onClicked: rpcClient.call("qmhrpc.takeAction", 5)
        }

        Button {
            id: context
            text: "Context"
            anchors.margins: 10
            anchors.top: down.bottom
            anchors.right: parent.right
            onClicked: rpcClient.call("qmhrpc.takeAction", 6)
        }


        Button {
            id: targets
            text: "Targets"
            anchors.margins: 10
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: rpcClient.disconnectFromHost()
        }
    }
}
