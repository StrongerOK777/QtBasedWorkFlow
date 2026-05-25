import QtQuick
import QtQuick.Controls

Rectangle {
    id: activityBar
    color: "#181818"
    border.color: "#2b2b2b"
    border.width: 1

    component ActivityButton: ToolButton {
        id: button
        property string sidebar: ""
        property string commandId: ""
        required property string iconName
        required property string hint
        property bool checkedState: sidebar.length > 0 && workbenchBridge.activeSidebar === sidebar
        width: parent ? parent.width : 48
        height: width
        hoverEnabled: true
        onClicked: {
            if (commandId.length > 0) {
                workbenchBridge.triggerCommand(commandId)
            } else if (sidebar.length > 0) {
                workbenchBridge.activeSidebar = sidebar
            }
        }
        WorkbenchTooltip {
            sourceItem: button
            active: button.hovered
            tooltipText: button.hint
            placement: "right"
        }
        background: Rectangle {
            color: button.hovered ? "#292929" : "transparent"
            Rectangle {
                width: 2
                height: parent.height
                visible: button.checkedState
                color: "#3794ff"
            }
        }
        contentItem: Item {
            WorkbenchIcon {
                anchors.centerIn: parent
                width: 30
                height: 30
                name: button.iconName
                strokeColor: button.checkedState ? "#f5f5f5" : "#a8a8a8"
                fillColor: "transparent"
                strokeWidth: 1.8
            }
        }
    }

    Column {
        anchors.fill: parent
        anchors.topMargin: 4

        ActivityButton {
            sidebar: "nodes"
            iconName: "nodes"
            hint: "节点库"
        }
        ActivityButton {
            sidebar: "workflow"
            iconName: "workflow"
            hint: "工作流大纲"
        }
        ActivityButton {
            sidebar: "templates"
            iconName: "templates"
            hint: "方案库"
        }
        ActivityButton {
            sidebar: "history"
            iconName: "source-control"
            hint: "进度记录"
        }
        ActivityButton {
            sidebar: "search"
            iconName: "search"
            hint: "搜索和快速访问"
        }
        ActivityButton {
            sidebar: "problems"
            iconName: "problems"
            hint: "问题"
        }
        ActivityButton {
            sidebar: "diagnostics"
            iconName: "run-diagnostics"
            hint: "运行诊断"
        }

        Item {
            width: parent.width
            height: Math.max(0, parent.height - y - 148)
        }

        ActivityButton {
            commandId: "workbench.quickAccess"
            iconName: "command"
            hint: "命令面板"
        }
        ActivityButton {
            commandId: "workbench.preview"
            iconName: "preview"
            hint: workbenchBridge.previewVisible ? "隐藏预览" : "显示预览"
            checkedState: workbenchBridge.previewVisible
        }
        ActivityButton {
            commandId: "settings.open"
            iconName: "settings"
            hint: "设置"
        }
    }
}
