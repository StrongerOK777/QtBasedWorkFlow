import QtQuick
import QtQuick.Controls

Rectangle {
    id: activityBar
    color: "#181818"
    border.color: "#2b2b2b"
    border.width: 1

    component ActivityButton: ToolButton {
        id: button
        required property string sidebar
        required property string glyph
        required property string hint
        width: parent ? parent.width : 48
        height: width
        hoverEnabled: true
        onClicked: workbenchBridge.activeSidebar = sidebar
        ToolTip.visible: hovered
        ToolTip.text: hint
        background: Rectangle {
            color: button.hovered ? "#292929" : "transparent"
            Rectangle {
                width: 2
                height: parent.height
                visible: workbenchBridge.activeSidebar === button.sidebar
                color: "#3794ff"
            }
        }
        contentItem: Text {
            text: button.glyph
            color: workbenchBridge.activeSidebar === button.sidebar ? "#f5f5f5" : "#a8a8a8"
            font.pixelSize: 22
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Column {
        anchors.fill: parent
        anchors.topMargin: 4

        ActivityButton {
            sidebar: "nodes"
            glyph: "+"
            hint: "节点库"
        }
        ActivityButton {
            sidebar: "workflow"
            glyph: "W"
            hint: "工作流"
        }
        ActivityButton {
            sidebar: "search"
            glyph: "/"
            hint: "搜索"
        }

        Item {
            width: parent.width
            height: Math.max(0, parent.height - y - 98)
        }

        ActivityButton {
            sidebar: "search"
            glyph: ">"
            hint: "命令面板"
            onClicked: workbenchBridge.showQuickAccess()
        }
        ActivityButton {
            sidebar: "nodes"
            glyph: "P"
            hint: workbenchBridge.previewVisible ? "隐藏预览" : "显示预览"
            onClicked: workbenchBridge.previewVisible = !workbenchBridge.previewVisible
        }
    }
}
