import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#252526"
    border.color: "#2d2d2d"
    border.width: 1

    component HeaderButton: ToolButton {
        id: button
        required property string iconName
        property string tooltip: ""
        width: 34
        height: 28
        padding: 0
        hoverEnabled: true
        ToolTip.visible: hovered && tooltip.length > 0
        ToolTip.text: tooltip
        background: Rectangle { color: button.hovered ? "#37373d" : "transparent" }
        contentItem: Item {
            WorkbenchIcon {
                anchors.centerIn: parent
                width: 18
                height: 18
                name: button.iconName
                strokeColor: "#cccccc"
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: "#1e1e1e"
            border.color: "#2d2d2d"
            Text {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 8
                text: workbenchBridge.documentTitle
                color: "#ffffff"
                font.pixelSize: 12
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
        }

        HeaderButton { iconName: "back"; tooltip: "Back"; onClicked: workbenchBridge.triggerCommand("macro.back") }
        HeaderButton { iconName: "forward"; tooltip: "Forward"; onClicked: workbenchBridge.triggerCommand("macro.forward") }
        HeaderButton { iconName: "add"; tooltip: "Add Node"; onClicked: workbenchBridge.triggerCommand("node.quickAdd") }
        HeaderButton { iconName: "layout"; tooltip: "Auto Layout"; onClicked: workbenchBridge.triggerCommand("workflow.autoLayout") }

        Item { Layout.fillWidth: true }

        HeaderButton { iconName: "preview"; tooltip: "Toggle Preview"; onClicked: workbenchBridge.previewVisible = !workbenchBridge.previewVisible }
        HeaderButton { iconName: "panel"; tooltip: "Toggle Panel"; onClicked: workbenchBridge.panelVisible = !workbenchBridge.panelVisible }
        Rectangle {
            Layout.preferredWidth: 86
            Layout.preferredHeight: 28
            color: runMouse.containsMouse ? "#16825d" : "#0e639c"
            Text {
                anchors.centerIn: parent
                text: "Run"
                color: "#ffffff"
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }
            MouseArea {
                id: runMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: workbenchBridge.triggerCommand("workflow.run")
            }
        }
    }
}
