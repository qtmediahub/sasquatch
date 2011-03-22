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
    property alias title : controlTitle.text

    clip: true
    state: "showingButtons"

    states: [
        State {
            name: "showingButtons"
            PropertyChanges {
                target: remoteControlButtons
                x: 0
            }
        },
        State {
            name: "showingTrackpad"
            PropertyChanges {
                target: remoteControlButtons
                x: -remoteControlButtons.width
            }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "x" }
    }

    Text {
        id: controlTitle
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        font.weight: Font.Light
        font.pointSize: 16
        color:  "#FCFCFC"
        style: Text.Sunken
        styleColor: "#000000"
        font.family: "Verdana"
    }

    VolumeControl {
        id: volumeControl
        anchors.top: controlTitle.bottom
        anchors.topMargin: 10
        width: parent.width
    }

    RemoteControlButtons {
        id: remoteControlButtons
        clip: true
        width: parent.width
        anchors.margins: 5
        anchors.top: volumeControl.bottom
        anchors.bottom: targets.top
        anchors.left: remoteControlButtons.right
    }

    Trackpad {
        id: trackpad
        width: parent.width
        anchors.margins: 5
        anchors.top: volumeControl.bottom
        anchors.bottom: targets.top
        anchors.left: remoteControlButtons.right
    }

    Button {
        id: targets
        text: qsTr("Targets")
        onClicked: rpcClient.disconnectFromHost()
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        width: root.width/2.0 - anchors.margins*2.0
    }

    Button {
        id: showTrackpad
        text: root.state == "showingTrackpad" ? qsTr("Buttons") : qsTr("Trackpad")
        onClicked: root.state = root.state == "showingTrackpad" ? "showingButtons" : "showingTrackpad"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        width: root.width/2.0 - anchors.margins*2.0
    }
}

