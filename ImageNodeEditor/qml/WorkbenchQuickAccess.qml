import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: quickAccess
    color: "#212327"
    border.color: "#34363b"
    border.width: 1
    radius: 12
    clip: true
    focus: true

    // 命令面板每次被唤起时淡入 + 轻微放大（从顶部生长），柔和登场。
    transformOrigin: Item.Top
    opacity: 0
    scale: 0.985
    ParallelAnimation {
        id: appearAnim
        NumberAnimation { target: quickAccess; property: "opacity"; to: 1; duration: 150; easing.type: Easing.OutCubic }
        NumberAnimation { target: quickAccess; property: "scale"; to: 1; duration: 180; easing.type: Easing.OutCubic }
    }
    Component.onCompleted: appearAnim.start()
    Connections {
        target: workbenchBridge
        function onQuickAccessRequested() {
            quickAccess.opacity = 0
            quickAccess.scale = 0.985
            appearAnim.restart()
        }
    }

    Keys.onEscapePressed: workbenchBridge.activateQuickAccess(-1)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TextField {
            id: query
            Layout.fillWidth: true
            height: 42
            placeholderText: "键入命令、节点、问题或最近工作流"
            placeholderTextColor: "#6b7178"
            color: "#e3e4e6"
            selectByMouse: true
            font.pixelSize: 15
            leftPadding: 14
            rightPadding: 14
            text: quickAccessModel.query
            onTextChanged: {
                quickAccessModel.query = text
                results.currentIndex = results.count > 0 ? 0 : -1
            }
            Keys.onDownPressed: results.incrementCurrentIndex()
            Keys.onUpPressed: results.decrementCurrentIndex()
            Keys.onEscapePressed: workbenchBridge.activateQuickAccess(-1)
            Keys.onReturnPressed: workbenchBridge.activateQuickAccess(results.currentIndex)
            Keys.onEnterPressed: workbenchBridge.activateQuickAccess(results.currentIndex)
            Component.onCompleted: forceActiveFocus()
            background: Rectangle {
                color: "transparent"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: query.activeFocus ? "#6ea0e0" : "#2e2f33"
                    Behavior on color { ColorAnimation { duration: 130; easing.type: Easing.OutCubic } }
                }
            }
        }

        ListView {
            id: results
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: quickAccessModel
            currentIndex: count > 0 ? 0 : -1
            keyNavigationWraps: true
            ScrollBar.vertical: ScrollBar {}
            delegate: Rectangle {
                required property int index
                required property string kind
                required property string title
                required property string detail
                width: ListView.view.width
                height: 48
                radius: 7
                color: ListView.isCurrentItem ? "#303a47" : resultMouse.containsMouse ? "#26282d" : "transparent"
                Behavior on color { ColorAnimation { duration: 110; easing.type: Easing.OutCubic } }

                Rectangle {
                    width: 68
                    height: 20
                    radius: 5
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#2c2f34"
                    border.color: "#2e2f33"
                    Text {
                        anchors.fill: parent
                        text: kind
                        color: "#a7adb3"
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 88
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    Text {
                        text: title
                        color: "#e3e4e6"
                        font.pixelSize: 13
                        width: parent.width
                        elide: Text.ElideRight
                    }
                    Text {
                        text: detail
                        color: "#9aa0a6"
                        font.pixelSize: 11
                        width: parent.width
                        elide: Text.ElideMiddle
                    }
                }
                MouseArea {
                    id: resultMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: results.currentIndex = index
                    onClicked: workbenchBridge.activateQuickAccess(index)
                }
            }
        }
    }
}
