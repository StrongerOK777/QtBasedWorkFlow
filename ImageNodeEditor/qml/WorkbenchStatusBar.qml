import QtQuick

Rectangle {
    color: "#18191c"

    // 顶部一条 hairline 分隔，融入整体而非刺眼的蓝条。
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#2e2f33"
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 16

        Text {
            text: workbenchBridge.documentTitle
            color: "#c8cace"
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Text {
            text: workbenchBridge.selectedNodeText
            color: "#9aa0a6"
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
            color: "#9aa0a6"
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Text {
            id: zoomText
            text: workbenchBridge.zoomText
            color: "#83a9d6"
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
    }
}
