import QtQuick

Rectangle {
    id: root
    color: theme.base

    function fs(px) { return Math.round(px * theme.scale) }

    // 顶部一条 hairline 分隔，融入整体而非刺眼的蓝条。
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: theme.hairline
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 16

        Text {
            text: workbenchBridge.documentTitle
            color: theme.textSecondary
            font.pixelSize: root.fs(13)
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Text {
            text: workbenchBridge.selectedNodeText
            color: theme.textSecondary
            font.pixelSize: root.fs(13)
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
            color: theme.textSecondary
            font.pixelSize: root.fs(13)
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
        Text {
            id: zoomText
            text: workbenchBridge.zoomText
            color: theme.accent
            font.pixelSize: root.fs(13)
            verticalAlignment: Text.AlignVCenter
            height: parent.height
        }
    }
}
