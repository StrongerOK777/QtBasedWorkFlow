import QtQuick
import QtQuick.Controls

Popup {
    id: tooltip
    required property Item sourceItem
    property bool active: false
    property string tooltipText: ""
    property string placement: "right"
    property int showDelay: 450
    property int edgeMargin: 8
    property bool ready: false

    parent: Overlay.overlay
    modal: false
    dim: false
    focus: false
    closePolicy: Popup.NoAutoClose
    visible: ready && active && tooltipText.length > 0
    padding: 0

    Timer {
        id: showTimer
        interval: tooltip.showDelay
        repeat: false
        onTriggered: tooltip.ready = true
    }

    onActiveChanged: {
        if (active) {
            ready = false
            showTimer.restart()
        } else {
            showTimer.stop()
            ready = false
        }
    }

    function sourcePoint() {
        if (!sourceItem || !parent) {
            return Qt.point(0, 0)
        }
        return sourceItem.mapToItem(parent, 0, 0)
    }

    x: {
        var p = sourcePoint()
        var next = placement === "bottom"
            ? p.x + (sourceItem.width - width) / 2
            : p.x + sourceItem.width + edgeMargin
        return Math.max(edgeMargin, Math.min(next, parent ? parent.width - width - edgeMargin : next))
    }

    y: {
        var p = sourcePoint()
        var next = placement === "bottom"
            ? p.y + sourceItem.height + edgeMargin
            : p.y + (sourceItem.height - height) / 2
        return Math.max(edgeMargin, Math.min(next, parent ? parent.height - height - edgeMargin : next))
    }

    background: Rectangle {
        color: "#252526"
        border.color: "#454545"
        border.width: 1
    }

    contentItem: Text {
        text: tooltip.tooltipText
        color: "#cccccc"
        font.pixelSize: 12
        lineHeight: 1.15
        wrapMode: Text.Wrap
        width: Math.min(260, Math.max(96, implicitWidth + 20))
        padding: 7
    }
}
