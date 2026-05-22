import QtQuick

Rectangle {
    color: "#007acc"

    Row {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 16

        Text {
            text: workbenchBridge.documentTitle
            color: "#ffffff"
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Text {
            text: workbenchBridge.selectedNodeText
            color: "#ffffff"
            opacity: 0.9
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Item {
            width: Math.max(0, parent.width - x - statusText.width - zoomText.width - 26)
            height: parent.height
        }
        Text {
            id: statusText
            text: workbenchBridge.statusText
            color: "#ffffff"
            opacity: 0.9
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Text {
            id: zoomText
            text: workbenchBridge.zoomText
            color: "#ffffff"
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
    }
}
