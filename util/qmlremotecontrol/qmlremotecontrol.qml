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
import RpcConnection 1.0

Rectangle {
    id: qmlRemote
    height: 640
    width: 360
    color: "black"

    state: "targets"

    Image {
        id: background
        anchors.fill: parent
        source: "qrc:/images/media-overlay.png"
    }

    RpcConnection {
        id: rpcClient
        property string source
        property int position

        onClientConnected: {
            qmlRemote.state = "control"
        }

        onClientDisconnected: {
            qmlRemote.state = "targets"
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
                rightMargin: -controlView.width
            }
            PropertyChanges {
                target: busyView
                state: ""
            }
        },
        State {
            name: "inProgress"
            PropertyChanges {
                target: busyView
                state: "visible"
            }
            PropertyChanges {
                target: controlView.anchors
                rightMargin: -controlView.width
            }
            PropertyChanges {
                target: targetsView.anchors
                leftMargin: -targetsView.width
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
                rightMargin: 0
            }
            PropertyChanges {
                target: busyView
                state: ""
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation { properties: "leftMargin, rightMargin"; }
        }
    ]

    TargetsView {
        id: targetsView

        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: parent.height
    }

    BusyView {
        id: busyView

        anchors.fill: parent
    }

    ControlView {
        id: controlView

        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width
        height: parent.height
    }
}
