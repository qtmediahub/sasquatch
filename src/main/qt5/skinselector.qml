/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

import QtQuick 2.5

FocusScope
{
    id: root

    QtObject {
        id: settings
        property real headerSize: root.width/20
        property real textSize: root.width/60
    }
    Item {
        id: header
        anchors.top: parent.top
        width: root.width
        height: childrenRect.height + 40
        Text {
            text: "Skin selector"
            font.pointSize: settings.headerSize
            anchors.centerIn: parent
        }
    }
    Rectangle {
        id: errorLog
        color: "red"
        visible: runtime.skinruntime.errorMsg != ""
        anchors.top: header.bottom
        width: parent.width
        height: errorMsg.height
        Text {
            id: errorMsg
            text: "Error output\n" + runtime.skinruntime.errorMsg
            font.pointSize: settings.textSize
        }
    }
    ListView {
        id: list
        focus: true

        anchors { top: errorLog.bottom; bottom: parent.bottom }

        model: runtime.skinManager.skinsModel

        highlight:
            Rectangle {
            width: root.width
            color: "lightsteelblue"; radius: 5
        }

        delegate:
            Rectangle {
            id: delegate
            width: ListView.view.width
            height: Math.max(column.height, screenshot.height)

            Image {
                id: screenshot
                anchors.verticalCenter: delegate.verticalCenter
                width: 75
                height: 50
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Column {
                id: column
                anchors {
                    topMargin: 5
                    bottomMargin: 5
                    leftMargin: 10
                    left: screenshot.right
                }
                spacing: 3
                Text {
                    id: skinName
                    font.bold: true
                    font.pointSize: settings.textSize
                }

                Text {
                    id: skinAuthors
                    font.pointSize: settings.textSize
                }

                Text {
                    id: website
                    font.pointSize: settings.textSize
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent

                onClicked: delegate.ListView.view.currentIndex = index
                onDoubleClicked: runtime.window.setSkin(model.modelData.name)
            }

            Keys.onEnterPressed: runtime.window.setSkin(model.modelData.name)
            Keys.onReturnPressed: runtime.window.setSkin(model.modelData.name)

            Component.onCompleted: {
                var doc = new XMLHttpRequest()
                doc.onreadystatechange = function() {
                    if (doc.readyState == XMLHttpRequest.DONE && doc.responseText) {
                        var manifest = eval('(' + doc.responseText + ')')
                        skinName.text = manifest.name + " (v" + manifest.version + ")"
                        if (manifest.screenshot)
                            screenshot.source = "file://" + model.modelData.path + "/" + manifest.screenshot
                        var authors = []
                        for (var i = 0; i < manifest.authors.length; i++) {
                            authors.push(manifest.authors[i].name)
                        }
                        skinAuthors.text = "By " + authors.join(", ")
                        website.text = manifest.website
                    }
                }
                doc.open("GET", "file://" + model.modelData.path + "/skin.manifest")
                doc.send()
            }
        }
    }
    Component.onCompleted:
        forceActiveFocus()
}
