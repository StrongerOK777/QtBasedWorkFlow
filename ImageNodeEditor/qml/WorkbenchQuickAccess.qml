import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: quickAccess
    color: theme.panel
    border.color: theme.border
    border.width: 1
    radius: 12
    clip: true
    focus: true

    function fs(px) { return Math.round(px * theme.scale) }

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
            placeholderTextColor: theme.textMuted
            color: theme.textPrimary
            selectByMouse: true
            font.pixelSize: quickAccess.fs(16)
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
                    color: query.activeFocus ? theme.accent : theme.hairline
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
                color: ListView.isCurrentItem ? theme.selection : resultMouse.containsMouse ? theme.elevated : "transparent"
                Behavior on color { ColorAnimation { duration: 110; easing.type: Easing.OutCubic } }

                Rectangle {
                    width: 68
                    height: 20
                    radius: 5
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    color: theme.input
                    border.color: theme.hairline
                    Text {
                        anchors.fill: parent
                        text: kind
                        color: theme.textSecondary
                        font.pixelSize: quickAccess.fs(11)
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
                        color: theme.textPrimary
                        font.pixelSize: quickAccess.fs(14)
                        width: parent.width
                        elide: Text.ElideRight
                    }
                    Text {
                        text: detail
                        color: theme.textSecondary
                        font.pixelSize: quickAccess.fs(12)
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
