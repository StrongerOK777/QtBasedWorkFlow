import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#212327"
    border.color: "#2e2f33"
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
        background: Rectangle {
            color: "transparent"
            Rectangle {
                anchors.fill: parent
                anchors.margins: 3
                radius: 6
                color: button.hovered ? "#2c2f34" : "transparent"
                Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
            }
        }
        contentItem: Item {
            WorkbenchIcon {
                anchors.centerIn: parent
                width: 18
                height: 18
                name: button.iconName
                strokeColor: button.hovered ? "#c8cace" : "#9aa0a6"
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: "#1b1c1e"
            Text {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 8
                text: workbenchBridge.documentTitle
                color: "#c8cace"
                font.pixelSize: 12.5
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
        }

        // 编辑器头部只作宏图层级面包屑：文档标题 + 前进/后退；
        // 执行/预览/底部面板/添加/整理 等命令统一收敛到顶部标题栏，避免右上角重复。
        HeaderButton { iconName: "back"; tooltipText: "返回上一级图"; onClicked: workbenchBridge.triggerCommand("macro.back") }
        HeaderButton { iconName: "forward"; tooltipText: "前进到下一级图"; onClicked: workbenchBridge.triggerCommand("macro.forward") }

        Item { Layout.fillWidth: true }
    }
}
