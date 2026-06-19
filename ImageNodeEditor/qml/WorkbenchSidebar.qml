import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    color: theme.panel
    border.color: theme.hairline
    border.width: 1

    // 字号统一跟随界面缩放：基准值已整体上调，避免文字过小看不清。
    function fs(px) { return Math.round(px * theme.scale) }

    component SidebarHeader: Text {
        color: theme.textMuted
        font.pixelSize: sidebar.fs(13)
        font.weight: Font.DemiBold
        font.letterSpacing: 0.4
        elide: Text.ElideRight
    }

    component SearchField: TextField {
        color: theme.textPrimary
        placeholderTextColor: theme.textMuted
        selectByMouse: true
        font.pixelSize: sidebar.fs(14)
        leftPadding: 10
        rightPadding: 10
        background: Rectangle {
            radius: 8
            color: theme.input
            border.color: parent.activeFocus ? theme.accent : theme.hairline
            border.width: 1
            Behavior on border.color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
        }
    }

    component ResultList: ListView {
        clip: true
        spacing: 4
        ScrollBar.vertical: ScrollBar {}
    }

    // 列表行内的小操作按钮（套用/恢复/改名/导出/删除等），统一外观与悬停反馈。
    component ActionChip: Rectangle {
        id: chip
        property string label
        property bool accent: false
        property bool danger: false
        signal activated()
        width: sidebar.fs(40)
        height: sidebar.fs(24)
        radius: 7
        color: chip.accent ? (chipMouse.containsMouse ? theme.accentHover : theme.accent)
                           : (chipMouse.containsMouse ? theme.elevatedHover : theme.elevated)
        border.color: chip.accent ? "transparent" : theme.border
        border.width: chip.accent ? 0 : 1
        Text {
            anchors.centerIn: parent
            text: chip.label
            color: chip.accent ? theme.onAccent : (chip.danger ? theme.danger : theme.textPrimary)
            font.pixelSize: sidebar.fs(12)
        }
        MouseArea {
            id: chipMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: chip.activated()
        }
    }

    StackLayout {
        anchors.fill: parent
        // 子面板物理顺序为 0 节点库 / 1 方案库 / 2 进度记录 / 3 工作流 / 4 搜索 / 5 问题 / 6 运行诊断，
        // currentIndex 必须与该顺序一致，否则会出现「点方案库显示进度记录」之类的错位。
        currentIndex: workbenchBridge.activeSidebar === "templates" ? 1
                      : workbenchBridge.activeSidebar === "history" ? 2
                      : workbenchBridge.activeSidebar === "workflow" ? 3
                      : workbenchBridge.activeSidebar === "search" ? 4
                      : workbenchBridge.activeSidebar === "problems" ? 5
                      : workbenchBridge.activeSidebar === "diagnostics" ? 6 : 0

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "节点库"
                    Layout.fillWidth: true
                }
                SearchField {
                    placeholderText: "搜索节点"
                    text: nodeCatalogModel.filterText
                    onTextChanged: nodeCatalogModel.filterText = text
                    Layout.fillWidth: true
                }
                ResultList {
                    model: nodeCatalogModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    // 节点选择按钮：统一风格的圆角矩形卡片（任务1c）。
                    delegate: Rectangle {
                        required property int index
                        required property string typeName
                        required property string title
                        required property string category
                        required property string provider
                        required property string iconName
                        x: 2
                        width: ListView.view.width - 4
                        height: sidebar.fs(48)
                        radius: 8
                        color: ListView.isCurrentItem ? theme.selection
                               : mouse.containsMouse ? theme.elevatedHover : theme.elevated
                        border.color: ListView.isCurrentItem ? theme.accent : theme.hairline
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 110; easing.type: Easing.OutCubic } }
                        property real pressX: 0
                        property real pressY: 0
                        property bool dragStarted: false
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 36
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: title
                                color: theme.textPrimary
                                font.pixelSize: sidebar.fs(14)
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: category + "  " + provider
                                color: theme.textMuted
                                font.pixelSize: sidebar.fs(12)
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Rectangle {
                            x: 10
                            width: 18
                            height: 18
                            anchors.verticalCenter: parent.verticalCenter
                            color: "transparent"
                            border.color: "transparent"
                            WorkbenchIcon {
                                anchors.centerIn: parent
                                width: 14
                                height: 14
                                name: iconName
                                strokeColor: theme.textSecondary
                                strokeWidth: 1.4
                            }
                        }
                        MouseArea {
                            id: mouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onPressed: {
                                parent.pressX = mouse.x
                                parent.pressY = mouse.y
                                parent.dragStarted = false
                            }
                            onPositionChanged: {
                                if (!pressed || parent.dragStarted)
                                    return
                                var dx = mouse.x - parent.pressX
                                var dy = mouse.y - parent.pressY
                                if (Math.sqrt(dx * dx + dy * dy) > 8) {
                                    parent.dragStarted = true
                                    workbenchBridge.startNodeDrag(typeName, title, category)
                                }
                            }
                            onClicked: {
                                // 单击只选中高亮，不再直接加节点；加节点请拖拽到画布。
                                parent.ListView.view.currentIndex = parent.index
                            }
                        }
                        WorkbenchTooltip {
                            sourceItem: mouse
                            active: mouse.containsMouse
                            tooltipText: "拖拽到画布添加：" + title
                            placement: "right"
                        }
                    }
                }
            }
        }

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "方案库"
                    Layout.fillWidth: true
                }
                Text {
                    text: "预设方案类似扩展面板入口，但只保存可直接套用的图像处理流程。"
                    color: theme.textMuted
                    font.pixelSize: sidebar.fs(13)
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: sidebar.fs(34)
                        radius: 8
                        color: templateSaveMouse.containsMouse ? theme.elevatedHover : theme.elevated
                        border.color: theme.border
                        Text {
                            anchors.centerIn: parent
                            text: "保存当前为模板"
                            color: theme.textPrimary
                            font.pixelSize: sidebar.fs(14)
                        }
                        MouseArea {
                            id: templateSaveMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: workbenchBridge.saveWorkflowTemplate()
                        }
                    }
                    Rectangle {
                        Layout.preferredWidth: sidebar.fs(88)
                        Layout.preferredHeight: sidebar.fs(34)
                        radius: 8
                        color: templateImportMouse.containsMouse ? theme.elevatedHover : theme.elevated
                        border.color: theme.border
                        Text {
                            anchors.centerIn: parent
                            text: "导入模板"
                            color: theme.textPrimary
                            font.pixelSize: sidebar.fs(14)
                        }
                        MouseArea {
                            id: templateImportMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: workbenchBridge.importWorkflowTemplate()
                        }
                    }
                }
                ResultList {
                    model: workflowTemplateModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string templateId
                        required property string title
                        required property string detail
                        required property string source
                        required property bool builtIn
                        width: ListView.view.width
                        height: 112
                        radius: 8
                        color: templateMouse.containsMouse ? theme.elevated : "transparent"
                        Rectangle {
                            x: 0
                            width: 3
                            height: parent.height
                            radius: 1.5
                            color: builtIn ? theme.accent : theme.textMuted
                        }
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.top: parent.top
                            anchors.topMargin: 7
                            spacing: 3
                            Text {
                                text: title
                                color: theme.textPrimary
                                font.pixelSize: sidebar.fs(14)
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: detail
                                color: theme.textSecondary
                                font.pixelSize: sidebar.fs(12)
                                maximumLineCount: 2
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: source
                                color: theme.textMuted
                                font.pixelSize: sidebar.fs(11)
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Row {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 6
                            spacing: 5
                            ActionChip {
                                label: "套用"
                                accent: true
                                width: sidebar.fs(44)
                                onActivated: workbenchBridge.applyWorkflowTemplate(templateId)
                            }
                            ActionChip {
                                label: "改名"
                                visible: !builtIn
                                onActivated: workbenchBridge.renameWorkflowTemplate(templateId)
                            }
                            ActionChip {
                                label: "导出"
                                onActivated: workbenchBridge.exportWorkflowTemplate(templateId)
                            }
                            ActionChip {
                                label: "删除"
                                danger: true
                                visible: !builtIn
                                onActivated: workbenchBridge.deleteWorkflowTemplate(templateId)
                            }
                        }
                        MouseArea {
                            id: templateMouse
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            hoverEnabled: true
                        }
                    }
                }
            }
        }

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "进度记录"
                    Layout.fillWidth: true
                }
                Text {
                    text: "像轻量版本管理一样保存当前画布、恢复旧版本，或从旧保存点创建新分支。"
                    color: theme.textMuted
                    font.pixelSize: sidebar.fs(13)
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }

                SidebarHeader {
                    text: "时间线"
                    Layout.fillWidth: true
                }
                Text {
                    text: "每次保存（Ctrl+S）自动记录，可点「恢复」回到那次保存。"
                    color: theme.textMuted
                    font.pixelSize: sidebar.fs(12)
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                ResultList {
                    model: workflowTimelineModel
                    Layout.fillWidth: true
                    Layout.preferredHeight: 156
                    delegate: Rectangle {
                        required property string checkpointId
                        required property string title
                        required property string detail
                        width: ListView.view.width
                        height: 50
                        radius: 8
                        color: timelineRowMouse.containsMouse ? theme.elevated : "transparent"
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: timelineRestore.left
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: title
                                color: theme.textPrimary
                                font.pixelSize: sidebar.fs(13)
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: detail
                                color: theme.textMuted
                                font.pixelSize: sidebar.fs(12)
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Rectangle {
                            id: timelineRestore
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            width: sidebar.fs(52)
                            height: sidebar.fs(24)
                            radius: 7
                            color: timelineRestoreMouse.containsMouse ? theme.accentHover : theme.accent
                            Text { anchors.centerIn: parent; text: "恢复"; color: theme.onAccent; font.pixelSize: sidebar.fs(13) }
                            MouseArea {
                                id: timelineRestoreMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: workbenchBridge.restoreTimeline(checkpointId)
                            }
                        }
                        MouseArea {
                            id: timelineRowMouse
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            hoverEnabled: true
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: theme.hairline
                }

                SidebarHeader {
                    text: "保存点"
                    Layout.fillWidth: true
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: sidebar.fs(34)
                        radius: 8
                        color: checkpointSaveMouse.containsMouse ? theme.elevatedHover : theme.elevated
                        border.color: theme.border
                        Text {
                            anchors.centerIn: parent
                            text: "保存当前进度"
                            color: theme.textPrimary
                            font.pixelSize: sidebar.fs(14)
                        }
                        MouseArea {
                            id: checkpointSaveMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: workbenchBridge.createCheckpoint()
                        }
                    }
                    Rectangle {
                        Layout.preferredWidth: sidebar.fs(72)
                        Layout.preferredHeight: sidebar.fs(34)
                        radius: 8
                        color: checkpointCompareMouse.containsMouse ? theme.elevatedHover : theme.elevated
                        border.color: theme.border
                        Text {
                            anchors.centerIn: parent
                            text: "对比…"
                            color: theme.textPrimary
                            font.pixelSize: sidebar.fs(14)
                        }
                        MouseArea {
                            id: checkpointCompareMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: workbenchBridge.compareCheckpoints()
                        }
                    }
                }
                ResultList {
                    model: workflowCheckpointModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string checkpointId
                        required property string title
                        required property string detail
                        required property string branch
                        width: ListView.view.width
                        height: 82
                        radius: 8
                        color: checkpointMouse.containsMouse ? theme.elevated : "transparent"
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.top: parent.top
                            anchors.topMargin: 7
                            spacing: 3
                            Text {
                                text: title
                                color: theme.textPrimary
                                font.pixelSize: sidebar.fs(14)
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: branch + "  ·  " + detail
                                color: theme.textMuted
                                font.pixelSize: sidebar.fs(12)
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Row {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 6
                            spacing: 4
                            ActionChip {
                                label: "恢复"
                                accent: true
                                width: sidebar.fs(36)
                                onActivated: workbenchBridge.restoreCheckpoint(checkpointId)
                            }
                            ActionChip {
                                label: "分支"
                                width: sidebar.fs(36)
                                onActivated: workbenchBridge.branchFromCheckpoint(checkpointId)
                            }
                            ActionChip {
                                label: "改名"
                                width: sidebar.fs(36)
                                onActivated: workbenchBridge.renameCheckpoint(checkpointId)
                            }
                            ActionChip {
                                label: "导出"
                                width: sidebar.fs(36)
                                onActivated: workbenchBridge.exportCheckpoint(checkpointId)
                            }
                            ActionChip {
                                label: "删除"
                                danger: true
                                width: sidebar.fs(36)
                                onActivated: workbenchBridge.deleteCheckpoint(checkpointId)
                            }
                        }
                        MouseArea {
                            id: checkpointMouse
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            hoverEnabled: true
                        }
                    }
                }
            }
        }

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "工作流"
                    Layout.fillWidth: true
                }
                SearchField {
                    placeholderText: "查找节点"
                    text: workflowOutlineModel.filterText
                    onTextChanged: workflowOutlineModel.filterText = text
                    Layout.fillWidth: true
                }
                ResultList {
                    model: workflowOutlineModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string nodeId
                        required property string title
                        required property string typeName
                        required property string category
                        x: 2
                        width: ListView.view.width - 4
                        height: sidebar.fs(46)
                        radius: 8
                        color: outlineMouse.containsMouse ? theme.elevated : "transparent"
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: title
                                color: theme.textPrimary
                                font.pixelSize: sidebar.fs(14)
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: {
                                    var i = nodeId.lastIndexOf("_");
                                    return i >= 0 ? category + "  #" + nodeId.substring(i + 1) : category;
                                }
                                color: theme.textMuted
                                font.pixelSize: sidebar.fs(12)
                                elide: Text.ElideMiddle
                                width: parent.width
                            }
                        }
                        MouseArea {
                            id: outlineMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: workbenchBridge.focusNode(nodeId)
                        }
                        WorkbenchTooltip {
                            sourceItem: outlineMouse
                            active: outlineMouse.containsMouse
                            tooltipText: "定位节点：" + title
                            placement: "right"
                        }
                    }
                }
            }
        }

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "快速访问"
                    Layout.fillWidth: true
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: sidebar.fs(34)
                    radius: 8
                    color: theme.input
                    border.color: quickMouse.containsMouse ? theme.accent : theme.hairline
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 9
                        text: "打开命令面板"
                        color: theme.textPrimary
                        font.pixelSize: sidebar.fs(14)
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: quickMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: workbenchBridge.showQuickAccess()
                    }
                }
                SidebarHeader {
                    text: "当前工作流"
                    Layout.topMargin: 8
                    Layout.fillWidth: true
                }
                ResultList {
                    model: workflowOutlineModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string nodeId
                        required property string title
                        required property string category
                        x: 2
                        width: ListView.view.width - 4
                        height: sidebar.fs(44)
                        radius: 8
                        color: searchMouse.containsMouse ? theme.elevated : "transparent"
                        Text {
                            anchors.fill: parent
                            anchors.margins: 8
                            text: title + "  ·  " + category
                            color: theme.textPrimary
                            font.pixelSize: sidebar.fs(13)
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                        MouseArea {
                            id: searchMouse
                            anchors.fill: parent
                            enabled: nodeId.length > 0
                            hoverEnabled: true
                            onClicked: workbenchBridge.focusNode(nodeId)
                        }
                    }
                }
            }
        }

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "问题"
                    Layout.fillWidth: true
                }
                ResultList {
                    model: problemModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string message
                        required property string nodeId
                        x: 2
                        width: ListView.view.width - 4
                        height: sidebar.fs(54)
                        radius: 8
                        color: problemMouse.containsMouse ? theme.elevated : "transparent"
                        Text {
                            anchors.fill: parent
                            anchors.margins: 8
                            text: message
                            color: theme.danger
                            font.pixelSize: sidebar.fs(13)
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                        MouseArea {
                            id: problemMouse
                            anchors.fill: parent
                            enabled: nodeId.length > 0
                            hoverEnabled: true
                            onClicked: workbenchBridge.focusNode(nodeId)
                        }
                    }
                }
            }
        }

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "运行诊断"
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 74
                    radius: 8
                    color: theme.panel
                    border.color: theme.border
                    Text {
                        anchors.fill: parent
                        anchors.margins: 8
                        text: workbenchBridge.statusText + "\n" + workbenchBridge.selectedNodeText + "\n" + workbenchBridge.zoomText
                        color: theme.textSecondary
                        font.pixelSize: sidebar.fs(13)
                        lineHeight: 1.15
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: sidebar.fs(34)
                    radius: 8
                    color: runDiagMouse.containsMouse ? theme.accentHover : theme.accent
                    Text {
                        anchors.centerIn: parent
                        text: "执行工作流"
                        color: theme.onAccent
                        font.pixelSize: sidebar.fs(14)
                        font.weight: Font.DemiBold
                    }
                    MouseArea {
                        id: runDiagMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: workbenchBridge.triggerCommand("workflow.run")
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: sidebar.fs(34)
                    radius: 8
                    color: layoutDiagMouse.containsMouse ? theme.elevatedHover : theme.elevated
                    border.color: theme.border
                    Text {
                        anchors.centerIn: parent
                        text: "整理画布"
                        color: theme.textPrimary
                        font.pixelSize: sidebar.fs(14)
                    }
                    MouseArea {
                        id: layoutDiagMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: workbenchBridge.triggerCommand("workflow.autoLayout")
                    }
                }

                SidebarHeader {
                    text: "最近问题"
                    Layout.topMargin: 8
                    Layout.fillWidth: true
                }
                ResultList {
                    model: problemModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string message
                        required property string nodeId
                        x: 2
                        width: ListView.view.width - 4
                        height: sidebar.fs(48)
                        radius: 8
                        color: diagProblemMouse.containsMouse ? theme.elevated : "transparent"
                        Text {
                            anchors.fill: parent
                            anchors.margins: 8
                            text: message
                            color: theme.danger
                            font.pixelSize: sidebar.fs(13)
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                        MouseArea {
                            id: diagProblemMouse
                            anchors.fill: parent
                            enabled: nodeId.length > 0
                            hoverEnabled: true
                            onClicked: workbenchBridge.focusNode(nodeId)
                        }
                    }
                }
            }
        }
    }
}
