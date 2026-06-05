import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: theme.panel
    border.color: theme.hairline
    border.width: 1

    function fs(px) { return Math.round(px * theme.scale) }

    component CommandButton: ToolButton {
        id: button
        required property string iconName
        property string tooltipText: ""
        property bool checkedState: false
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
                color: button.checkedState ? theme.selection : (button.hovered || button.down) ? theme.elevated : "transparent"
                border.color: button.checkedState ? theme.accent : "transparent"
                border.width: button.checkedState ? 1 : 0
                Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
            }
        }
        contentItem: Item {
            WorkbenchIcon {
                anchors.centerIn: parent
                width: 19
                height: 19
                name: button.iconName
                strokeColor: button.checkedState ? theme.accent : button.enabled ? (button.hovered ? theme.textPrimary : theme.textSecondary) : theme.textMuted
                strokeWidth: 1.7
                Behavior on strokeColor { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item { Layout.preferredWidth: Qt.platform.os === "osx" ? 78 : 8 }

        CommandButton {
            iconName: "back"
            tooltipText: "返回上一级图"
            onClicked: workbenchBridge.triggerCommand("macro.back")
        }
        CommandButton {
            iconName: "forward"
            tooltipText: "前进到下一级图"
            onClicked: workbenchBridge.triggerCommand("macro.forward")
        }
        CommandButton {
            iconName: "add"
            tooltipText: "快速添加节点"
            onClicked: workbenchBridge.triggerCommand("node.quickAdd")
        }
        CommandButton {
            iconName: "layout"
            tooltipText: "整理画布"
            onClicked: workbenchBridge.triggerCommand("workflow.autoLayout")
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            Layout.preferredWidth: Math.min(520, Math.max(280, root.width * 0.34))
            Layout.preferredHeight: 26
            radius: 8
            color: commandCenterMouse.containsMouse ? theme.input : theme.elevated
            border.color: commandCenterMouse.containsMouse ? theme.accent : theme.border
            Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
            Behavior on border.color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
            Text {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                text: workbenchBridge.documentTitle
                color: theme.textSecondary
                font.pixelSize: root.fs(13)
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
            WorkbenchTooltip {
                sourceItem: commandCenterMouse
                active: commandCenterMouse.containsMouse
                tooltipText: "搜索 / 命令 / 添加节点"
                placement: "bottom"
            }
            MouseArea {
                id: commandCenterMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: workbenchBridge.showQuickAccess()
            }
        }

        Item { Layout.fillWidth: true }

        CommandButton {
            iconName: "run"
            tooltipText: "执行工作流"
            onClicked: workbenchBridge.triggerCommand("workflow.run")
        }
        CommandButton {
            iconName: "preview"
            tooltipText: workbenchBridge.previewVisible ? "隐藏预览" : "显示预览"
            checkedState: workbenchBridge.previewVisible
            onClicked: workbenchBridge.previewVisible = !workbenchBridge.previewVisible
        }
        CommandButton {
            iconName: "panel"
            tooltipText: workbenchBridge.panelVisible ? "隐藏底部面板" : "显示底部面板"
            checkedState: workbenchBridge.panelVisible
            onClicked: workbenchBridge.panelVisible = !workbenchBridge.panelVisible
        }

        // 窗口控制按钮：macOS 复用左上角原生红绿灯，这里只在其它平台显示。
        CommandButton {
            iconName: "window-minimize"
            tooltipText: "最小化"
            visible: Qt.platform.os !== "osx"
            onClicked: workbenchBridge.requestWindowMinimize()
        }
        CommandButton {
            iconName: workbenchBridge.windowMaximized ? "window-restore" : "window-maximize"
            tooltipText: workbenchBridge.windowMaximized ? "向下还原" : "最大化"
            visible: Qt.platform.os !== "osx"
            onClicked: workbenchBridge.requestWindowMaximizeToggle()
        }
        CommandButton {
            iconName: "window-close"
            tooltipText: "关闭"
            visible: Qt.platform.os !== "osx"
            onClicked: workbenchBridge.requestWindowClose()
        }

        Item { Layout.preferredWidth: 8 }
    }

    // 标题栏空白处：拖动移动窗口、双击切换最大化。两枚 Handler 会让位给上方的
    // 命令按钮和命令框（它们各自的 MouseArea 会先抢占指针），只在空白区域生效。
    DragHandler {
        target: null
        onActiveChanged: if (active) workbenchBridge.requestWindowMove()
    }
    TapHandler {
        gesturePolicy: TapHandler.DragThreshold
        onDoubleTapped: workbenchBridge.requestWindowMaximizeToggle()
    }
}
