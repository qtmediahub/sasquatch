import QtQuick 1.0

Rectangle {
    id: addTargetDialog
    width: grid.width + 20
    height: grid.height + 20
    anchors.centerIn: parent
    color: "gray"

    Grid {
        id: grid
        columns: 2
        rows: 4
        spacing: 4
        anchors.centerIn: parent

        Text { text: qsTr("Hostname:"); color: "white"  } 
        TextInput { id: hostNameInput; focus: true; width: 100 }
        Text { text: qsTr("IP:"); color: "white" }
        TextInput { id: ipInput; width: 100 }
        Text { text: qsTr("Port:"); color: "white" }
        TextInput { id: portInput; width: 100; text: "1234"; validator: IntValidator { top: 65535; bottom: 1 } }

        Button { 
            id: okButton; text: qsTr("OK"); 
            onClicked: { 
                targetsModel.addService(hostNameInput.text, ipInput.text, portInput.text) 
                addTargetDialog.opacity = 0
            }
        }
        Button { id: cancelButton; text: qsTr("Cancel"); onClicked: addTargetDialog.opacity = 0 }
    }
}

