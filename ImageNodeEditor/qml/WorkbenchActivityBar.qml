import QtQuick
import QtQuick.Controls

Rectangle {
    id: activityBar
    color: "#1b1c1e"
    border.color: "#2e2f33"
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
            color: "transparent"
            Rectangle {
                anchors.fill: parent
                anchors.margins: 6
                radius: 9
                color: button.checkedState ? "#2a313b" : button.hovered ? "#26282d" : "transparent"
                Behavior on color { ColorAnimation { duration: 130; easing.type: Easing.OutCubic } }
            }
            Rectangle {
                width: 3
                radius: 1.5
                height: parent.height * 0.42
                anchors.verticalCenter: parent.verticalCenter
                x: 1
                visible: button.checkedState
                color: "#6ea0e0"
            }
        }
        contentItem: Item {
            WorkbenchIcon {
                anchors.centerIn: parent
                width: 28
                height: 28
                name: button.iconName
                strokeColor: button.checkedState ? "#e8eaed" : button.hovered ? "#c8cace" : "#8d9298"
                fillColor: "transparent"
                strokeWidth: 1.7
                Behavior on strokeColor { ColorAnimation { duration: 130; easing.type: Easing.OutCubic } }
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
