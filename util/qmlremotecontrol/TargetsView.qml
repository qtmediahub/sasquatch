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
        anchors.bottom: root.bottom
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
                    controlView.title = "Connected to "+model.display
                    qmlRemote.state = "inProgress"
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
