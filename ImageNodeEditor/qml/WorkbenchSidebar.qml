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
        currentIndex: workbenchBridge.activeSidebar === "workflow" ? 1
                      : workbenchBridge.activeSidebar === "search" ? 2 : 0

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                SidebarHeader {
                    text: "NODE LIBRARY"
                    Layout.fillWidth: true
                }
                SearchField {
                    placeholderText: "Search nodes"
                    text: nodeCatalogModel.filterText
                    onTextChanged: nodeCatalogModel.filterText = text
                    Layout.fillWidth: true
                }
                ResultList {
                    model: nodeCatalogModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        required property string typeName
                        required property string title
                        required property string category
                        required property string provider
                        required property string iconName
                        width: ListView.view.width
                        height: 52
                        color: mouse.containsMouse ? "#2a2d2e" : "transparent"
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
                            color: "#333333"
                            border.color: "#555555"
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
                                if (!parent.dragStarted)
                                    workbenchBridge.createNode(typeName)
                            }
                            onDoubleClicked: workbenchBridge.createNode(typeName)
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
                    text: "WORKFLOW"
                    Layout.fillWidth: true
                }
                SearchField {
                    placeholderText: "Find node"
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
                        height: 52
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
                                text: category + "  " + nodeId
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
                    text: "QUICK ACCESS"
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
                        text: "Open Command Palette"
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
                    text: "PROBLEMS"
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
                        height: 50
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
    }
}
