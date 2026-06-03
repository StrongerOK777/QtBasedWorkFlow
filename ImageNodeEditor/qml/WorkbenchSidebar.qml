import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    color: "#202020"
    border.color: "#303030"
    border.width: 1

    component SidebarHeader: Text {
        color: "#bbbbbb"
        font.pixelSize: 11
        font.weight: Font.Bold
        font.capitalization: Font.AllUppercase
        elide: Text.ElideRight
    }

    component SearchField: TextField {
        color: "#f0f0f0"
        placeholderTextColor: "#989898"
        selectByMouse: true
        font.pixelSize: 13
        leftPadding: 8
        rightPadding: 8
        background: Rectangle {
            color: "#313131"
            border.color: parent.activeFocus ? "#3794ff" : "#454545"
            border.width: 1
        }
    }

    component ResultList: ListView {
        clip: true
        spacing: 1
        ScrollBar.vertical: ScrollBar {}
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
                    delegate: Rectangle {
                        required property int index
                        required property string typeName
                        required property string title
                        required property string category
                        required property string provider
                        required property string iconName
                        width: ListView.view.width
                        height: 46
                        color: ListView.isCurrentItem ? "#094771"
                               : mouse.containsMouse ? "#2a2d2e" : "transparent"
                        property real pressX: 0
                        property real pressY: 0
                        property bool dragStarted: false
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 34
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: title
                                color: "#dddddd"
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: category + "  " + provider
                                color: "#969696"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Rectangle {
                            x: 8
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
                                strokeColor: "#cccccc"
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
                    color: "#969696"
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 34
                    color: templateSaveMouse.containsMouse ? "#2a2d2e" : "#252526"
                    border.color: "#3c3c3c"
                    Text {
                        anchors.centerIn: parent
                        text: "保存当前为模板"
                        color: "#cccccc"
                        font.pixelSize: 13
                    }
                    MouseArea {
                        id: templateSaveMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: workbenchBridge.saveWorkflowTemplate()
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
                        height: 72
                        color: templateMouse.containsMouse ? "#2a2d2e" : "transparent"
                        Rectangle {
                            x: 0
                            width: 2
                            height: parent.height
                            color: builtIn ? "#3794ff" : "#858585"
                        }
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: applyTemplateButton.left
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 3
                            Text {
                                text: title
                                color: "#dddddd"
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: detail
                                color: "#a8a8a8"
                                font.pixelSize: 11
                                maximumLineCount: 2
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: source
                                color: "#6f6f6f"
                                font.pixelSize: 10
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Rectangle {
                            id: applyTemplateButton
                            anchors.right: parent.right
                            anchors.rightMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            width: 44
                            height: 26
                            color: applyTemplateMouse.containsMouse ? "#0e639c" : "#313131"
                            border.color: applyTemplateMouse.containsMouse ? "#3794ff" : "#454545"
                            Text {
                                anchors.centerIn: parent
                                text: "套用"
                                color: "#ffffff"
                                font.pixelSize: 12
                            }
                            MouseArea {
                                id: applyTemplateMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: workbenchBridge.applyWorkflowTemplate(templateId)
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
                    color: "#969696"
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }

                SidebarHeader {
                    text: "时间线"
                    Layout.fillWidth: true
                }
                Text {
                    text: "每次保存（Ctrl+S）自动记录，可点「恢复」回到那次保存。"
                    color: "#969696"
                    font.pixelSize: 11
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
                        color: timelineRowMouse.containsMouse ? "#2a2d2e" : "transparent"
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: timelineRestore.left
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: title
                                color: "#dddddd"
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: detail
                                color: "#969696"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Rectangle {
                            id: timelineRestore
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            width: 52
                            height: 24
                            color: timelineRestoreMouse.containsMouse ? "#0e639c" : "#313131"
                            border.color: timelineRestoreMouse.containsMouse ? "#3794ff" : "#454545"
                            Text { anchors.centerIn: parent; text: "恢复"; color: "#ffffff"; font.pixelSize: 12 }
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
                    color: "#333333"
                }

                SidebarHeader {
                    text: "保存点"
                    Layout.fillWidth: true
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 34
                    color: checkpointSaveMouse.containsMouse ? "#2a2d2e" : "#252526"
                    border.color: "#3c3c3c"
                    Text {
                        anchors.centerIn: parent
                        text: "保存当前进度"
                        color: "#cccccc"
                        font.pixelSize: 13
                    }
                    MouseArea {
                        id: checkpointSaveMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: workbenchBridge.createCheckpoint()
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
                        color: checkpointMouse.containsMouse ? "#2a2d2e" : "transparent"
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
                                color: "#dddddd"
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: branch + "  ·  " + detail
                                color: "#969696"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }
                        Row {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 6
                            spacing: 6
                            Rectangle {
                                width: 52
                                height: 24
                                color: restoreMouse.containsMouse ? "#0e639c" : "#313131"
                                border.color: restoreMouse.containsMouse ? "#3794ff" : "#454545"
                                Text { anchors.centerIn: parent; text: "恢复"; color: "#ffffff"; font.pixelSize: 12 }
                                MouseArea {
                                    id: restoreMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: workbenchBridge.restoreCheckpoint(checkpointId)
                                }
                            }
                            Rectangle {
                                width: 52
                                height: 24
                                color: branchMouse.containsMouse ? "#3a3d41" : "#252526"
                                border.color: "#454545"
                                Text { anchors.centerIn: parent; text: "分支"; color: "#cccccc"; font.pixelSize: 12 }
                                MouseArea {
                                    id: branchMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: workbenchBridge.branchFromCheckpoint(checkpointId)
                                }
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
                        width: ListView.view.width
                        height: 46
                        color: outlineMouse.containsMouse ? "#2a2d2e" : "transparent"
                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: title
                                color: "#dddddd"
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: {
                                    var i = nodeId.lastIndexOf("_");
                                    return i >= 0 ? category + "  #" + nodeId.substring(i + 1) : category;
                                }
                                color: "#969696"
                                font.pixelSize: 11
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
                    height: 34
                    color: "#313131"
                    border.color: quickMouse.containsMouse ? "#3794ff" : "#454545"
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 9
                        text: "打开命令面板"
                        color: "#dddddd"
                        font.pixelSize: 13
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
                        width: ListView.view.width
                        height: 44
                        color: searchMouse.containsMouse ? "#2a2d2e" : "transparent"
                        Text {
                            anchors.fill: parent
                            anchors.margins: 6
                            text: title + "  ·  " + category
                            color: "#dddddd"
                            font.pixelSize: 12
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
                        width: ListView.view.width
                        height: 54
                        color: problemMouse.containsMouse ? "#2a2d2e" : "transparent"
                        Text {
                            anchors.fill: parent
                            anchors.margins: 6
                            text: message
                            color: "#f1b2ae"
                            font.pixelSize: 12
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
                    color: "#252526"
                    border.color: "#3c3c3c"
                    Text {
                        anchors.fill: parent
                        anchors.margins: 8
                        text: workbenchBridge.statusText + "\n" + workbenchBridge.selectedNodeText + "\n" + workbenchBridge.zoomText
                        color: "#cccccc"
                        font.pixelSize: 12
                        lineHeight: 1.15
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 34
                    color: runDiagMouse.containsMouse ? "#0e639c" : "#094771"
                    border.color: "#3794ff"
                    Text {
                        anchors.centerIn: parent
                        text: "执行工作流"
                        color: "#ffffff"
                        font.pixelSize: 13
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
                    height: 34
                    color: layoutDiagMouse.containsMouse ? "#2a2d2e" : "#252526"
                    border.color: "#3c3c3c"
                    Text {
                        anchors.centerIn: parent
                        text: "整理画布"
                        color: "#cccccc"
                        font.pixelSize: 13
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
                        width: ListView.view.width
                        height: 48
                        color: diagProblemMouse.containsMouse ? "#2a2d2e" : "transparent"
                        Text {
                            anchors.fill: parent
                            anchors.margins: 6
                            text: message
                            color: "#f1b2ae"
                            font.pixelSize: 12
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
