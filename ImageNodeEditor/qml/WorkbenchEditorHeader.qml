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
        property string tooltipText: ""
        width: 34
        height: 28
        padding: 0
        hoverEnabled: true
        WorkbenchTooltip {
            sourceItem: button
            active: button.hovered
            tooltipText: button.tooltipText
            placement: "bottom"
        }
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

        HeaderButton { iconName: "back"; tooltipText: "返回上一级图"; onClicked: workbenchBridge.triggerCommand("macro.back") }
        HeaderButton { iconName: "forward"; tooltipText: "前进到下一级图"; onClicked: workbenchBridge.triggerCommand("macro.forward") }
        HeaderButton { iconName: "add"; tooltipText: "添加节点"; onClicked: workbenchBridge.triggerCommand("node.quickAdd") }
        HeaderButton { iconName: "layout"; tooltipText: "整理画布"; onClicked: workbenchBridge.triggerCommand("workflow.autoLayout") }

        Item { Layout.fillWidth: true }

        HeaderButton { iconName: "preview"; tooltipText: "显示或隐藏预览"; onClicked: workbenchBridge.previewVisible = !workbenchBridge.previewVisible }
        HeaderButton { iconName: "panel"; tooltipText: "显示或隐藏底部面板"; onClicked: workbenchBridge.panelVisible = !workbenchBridge.panelVisible }
        Rectangle {
            Layout.preferredWidth: 86
            Layout.preferredHeight: 28
            color: runMouse.containsMouse ? "#16825d" : "#0e639c"
            Text {
                anchors.centerIn: parent
                text: "执行"
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
            WorkbenchTooltip {
                sourceItem: runMouse
                active: runMouse.containsMouse
                tooltipText: "执行工作流"
                placement: "bottom"
            }
        }
    }
}
