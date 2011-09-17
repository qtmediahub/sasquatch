import QtQuick 1.1

ListView {
    id: root
    focus: true

    SystemPalette { 
        id: palette
        colorGroup: SystemPalette.Active 
    }

    anchors.fill: parent
    model: skinManager.skinsModel

    delegate: Rectangle {
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
            onDoubleClicked: window.setSkin(model.modelData.name)
        }

        Keys.onEnterPressed: window.setSkin(model.modelData.name)
        Keys.onReturnPressed: window.setSkin(model.modelData.name)

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

