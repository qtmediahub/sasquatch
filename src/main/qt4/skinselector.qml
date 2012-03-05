import QtQuick 1.1

FocusScope
{
    id: root
    Item {
        id: header
        anchors.top: parent.top
        width: parent.width
        height: 80
        Text {
            text: "Skin selector"
            font.pointSize: 40
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
            font.pointSize: 12
        }
    }
    ListView {
        id: list
        focus: true
        width: 800
        height: 600

        SystemPalette {
            id: palette
            colorGroup: SystemPalette.Active
        }

        anchors { top: errorLog.bottom; bottom: parent.bottom }

            model: runtime.skinManager.skinsModel

            delegate:
                Rectangle {
                id: delegate
                width: ListView.view.width
                height: Math.max(column.height, screenshot.height)
                color: ListView.isCurrentItem ? palette.highlight : "transparent"

                Image {
                    id: screenshot
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
                        color: delegate.ListView.view.isCurrentItem ? palette.highlightedText : palette.text
                    }

                    Text {
                        id: skinAuthors
                        font.pointSize: skinName.font.pointSize - 4
                    }

                    Text {
                        id: website
                        font.pointSize: skinName.font.pointSize - 4
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
