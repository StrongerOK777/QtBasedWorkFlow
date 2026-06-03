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

        // 编辑器头部只作宏图层级面包屑：文档标题 + 前进/后退；
        // 执行/预览/底部面板/添加/整理 等命令统一收敛到顶部标题栏，避免右上角重复。
        HeaderButton { iconName: "back"; tooltipText: "返回上一级图"; onClicked: workbenchBridge.triggerCommand("macro.back") }
        HeaderButton { iconName: "forward"; tooltipText: "前进到下一级图"; onClicked: workbenchBridge.triggerCommand("macro.forward") }

        Item { Layout.fillWidth: true }
    }
}
