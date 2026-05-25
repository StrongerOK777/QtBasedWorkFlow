import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: quickAccess
    color: "#252526"
    border.color: "#454545"
    border.width: 1
    focus: true

    Keys.onEscapePressed: workbenchBridge.activateQuickAccess(-1)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TextField {
            id: query
            Layout.fillWidth: true
            height: 42
            placeholderText: "键入命令、节点、问题或最近工作流"
            placeholderTextColor: "#999999"
            color: "#f2f2f2"
            selectByMouse: true
            font.pixelSize: 15
            leftPadding: 12
            rightPadding: 12
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
                color: "#3c3c3c"
                border.color: query.activeFocus ? "#3794ff" : "#555555"
                border.width: 1
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
                color: ListView.isCurrentItem ? "#04395e" : resultMouse.containsMouse ? "#2a2d2e" : "transparent"

                Rectangle {
                    width: 68
                    height: 20
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#333333"
                    border.color: "#4f4f4f"
                    Text {
                        anchors.fill: parent
                        text: kind
                        color: "#cfcfcf"
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
                        color: "#f2f2f2"
                        font.pixelSize: 13
                        width: parent.width
                        elide: Text.ElideRight
                    }
                    Text {
                        text: detail
                        color: "#9f9f9f"
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
