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
        required property string iconName
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
        contentItem: Item {
            WorkbenchIcon {
                anchors.centerIn: parent
                width: 23
                height: 23
                name: button.iconName
                strokeColor: workbenchBridge.activeSidebar === button.sidebar ? "#f5f5f5" : "#a8a8a8"
                fillColor: "transparent"
            }
        }
    }

    Column {
        anchors.fill: parent
        anchors.topMargin: 4

        ActivityButton {
            sidebar: "nodes"
            iconName: "nodes"
            hint: "Node Library"
        }
        ActivityButton {
            sidebar: "workflow"
            iconName: "workflow"
            hint: "Workflow"
        }
        ActivityButton {
            sidebar: "search"
            iconName: "search"
            hint: "Search"
        }

        Item {
            width: parent.width
            height: Math.max(0, parent.height - y - 98)
        }

        ActivityButton {
            sidebar: "search"
            iconName: "command"
            hint: "Command Palette"
            onClicked: workbenchBridge.showQuickAccess()
        }
        ActivityButton {
            sidebar: "nodes"
            iconName: "preview"
            hint: workbenchBridge.previewVisible ? "Hide Preview" : "Show Preview"
            onClicked: workbenchBridge.previewVisible = !workbenchBridge.previewVisible
        }
    }
}
