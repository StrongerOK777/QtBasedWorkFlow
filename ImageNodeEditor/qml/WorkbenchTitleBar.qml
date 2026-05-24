import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#3c3c3c"
    border.color: "#2b2b2b"
    border.width: 1

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: workbenchBridge.requestWindowMove()
        onDoubleClicked: workbenchBridge.requestWindowMaximizeToggle()
    }

    component MenuButton: ToolButton {
        id: button
        required property string label
        property Menu menuPopup
        text: label
        hoverEnabled: true
        font.pixelSize: 12
        padding: 0
        implicitWidth: Math.max(42, contentItem.implicitWidth + 18)
        implicitHeight: 30
        background: Rectangle { color: button.hovered || button.down ? "#505050" : "transparent" }
        contentItem: Text {
            text: button.text
            color: "#cccccc"
            font: button.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        onClicked: if (menuPopup) menuPopup.open()
    }

    component DarkMenu: Menu {
        id: menu
        padding: 4
        background: Rectangle {
            color: "#252526"
            border.color: "#454545"
            border.width: 1
        }
    }

    component DarkMenuItem: MenuItem {
        id: item
        implicitWidth: 220
        implicitHeight: 28
        background: Rectangle { color: item.highlighted ? "#04395e" : "transparent" }
        contentItem: Text {
            text: item.text
            color: item.enabled ? "#cccccc" : "#6a6a6a"
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component DarkMenuSeparator: MenuSeparator {
        contentItem: Rectangle {
            implicitHeight: 1
            color: "#454545"
        }
    }

    component ChromeButton: ToolButton {
        id: button
        required property string glyph
        property bool closeButton: false
        text: glyph
        width: 46
        height: parent ? parent.height : 35
        padding: 0
        hoverEnabled: true
        background: Rectangle {
            color: button.hovered ? (button.closeButton ? "#e81123" : "#505050") : "transparent"
        }
        contentItem: Text {
            text: button.text
            color: button.hovered && button.closeButton ? "#ffffff" : "#cccccc"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 48
            Layout.fillHeight: true
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "INE"
                color: "#cccccc"
                font.pixelSize: 11
                font.weight: Font.DemiBold
            }
        }

        MenuButton {
            label: "File"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "New"; onTriggered: workbenchBridge.triggerCommand("file.new") }
                DarkMenuItem { text: "Open..."; onTriggered: workbenchBridge.triggerCommand("file.open") }
                DarkMenuItem { text: "Save"; onTriggered: workbenchBridge.triggerCommand("file.save") }
                DarkMenuItem { text: "Save As..."; onTriggered: workbenchBridge.triggerCommand("file.saveAs") }
                DarkMenuSeparator {}
                DarkMenuItem { text: "Export Workflow"; onTriggered: workbenchBridge.triggerCommand("file.exportWorkflow") }
                DarkMenuItem { text: "Export Preview"; onTriggered: workbenchBridge.triggerCommand("file.exportPreview") }
                DarkMenuItem { text: "Export Canvas"; onTriggered: workbenchBridge.triggerCommand("file.exportCanvas") }
            }
        }
        MenuButton {
            label: "Edit"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Undo"; onTriggered: workbenchBridge.triggerCommand("edit.undo") }
                DarkMenuItem { text: "Redo"; onTriggered: workbenchBridge.triggerCommand("edit.redo") }
            }
        }
        MenuButton {
            label: "Selection"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Quick Add Node"; onTriggered: workbenchBridge.triggerCommand("node.quickAdd") }
            }
        }
        MenuButton {
            label: "Node"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Quick Add Node"; onTriggered: workbenchBridge.triggerCommand("node.quickAdd") }
                DarkMenuItem { text: "Encapsulate as Macro"; onTriggered: workbenchBridge.triggerCommand("macro.encapsulate") }
                DarkMenuItem { text: "Auto Layout"; onTriggered: workbenchBridge.triggerCommand("workflow.autoLayout") }
            }
        }
        MenuButton {
            label: "View"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Command Palette"; onTriggered: workbenchBridge.triggerCommand("workbench.quickAccess") }
                DarkMenuSeparator {}
                DarkMenuItem { text: "Zoom In"; onTriggered: workbenchBridge.triggerCommand("view.zoomIn") }
                DarkMenuItem { text: "Zoom Out"; onTriggered: workbenchBridge.triggerCommand("view.zoomOut") }
                DarkMenuItem { text: "Reset Zoom"; onTriggered: workbenchBridge.triggerCommand("view.resetScale") }
                DarkMenuSeparator {}
                DarkMenuItem { text: "Toggle Preview"; onTriggered: workbenchBridge.triggerCommand("workbench.preview") }
                DarkMenuItem { text: "Toggle Panel"; onTriggered: workbenchBridge.triggerCommand("workbench.panel") }
                DarkMenuItem { text: "Reset Layout"; onTriggered: workbenchBridge.triggerCommand("workbench.resetLayout") }
            }
        }
        MenuButton {
            label: "Run"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Run Workflow"; onTriggered: workbenchBridge.triggerCommand("workflow.run") }
            }
        }
        MenuButton {
            label: "Terminal"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Focus Panel"; onTriggered: workbenchBridge.panelVisible = true }
            }
        }
        MenuButton {
            label: "Help"
            menuPopup: DarkMenu {
                DarkMenuItem { text: "Command Palette"; onTriggered: workbenchBridge.showQuickAccess() }
            }
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            Layout.preferredWidth: 420
            Layout.preferredHeight: 24
            color: "#2f2f2f"
            border.color: "#555555"
            Text {
                anchors.centerIn: parent
                text: workbenchBridge.documentTitle
                color: "#cccccc"
                font.pixelSize: 12
                elide: Text.ElideMiddle
            }
            MouseArea {
                anchors.fill: parent
                onClicked: workbenchBridge.showQuickAccess()
            }
        }

        Item { Layout.fillWidth: true }

        ChromeButton { glyph: "-"; onClicked: workbenchBridge.requestWindowMinimize() }
        ChromeButton { glyph: "[]"; onClicked: workbenchBridge.requestWindowMaximizeToggle() }
        ChromeButton { glyph: "x"; closeButton: true; onClicked: workbenchBridge.requestWindowClose() }
    }
}
