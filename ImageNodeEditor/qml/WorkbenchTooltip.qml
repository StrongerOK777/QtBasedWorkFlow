import QtQuick

Item {
    id: tooltip

    required property Item sourceItem
    property bool active: false
    property string tooltipText: ""
    property string placement: "right"
    property int showDelay: 450

    width: 0
    height: 0
    visible: false

    Timer {
        id: showTimer
        interval: tooltip.showDelay
        repeat: false
        onTriggered: {
            if (tooltip.active && tooltip.tooltipText.length > 0) {
                workbenchBridge.showTooltip(tooltip.tooltipText, tooltip.placement)
            }
        }
    }

    onActiveChanged: {
        if (active && tooltipText.length > 0) {
            showTimer.restart()
        } else {
            showTimer.stop()
            workbenchBridge.hideTooltip()
        }
    }

    onTooltipTextChanged: {
        if (active) {
            showTimer.restart()
        }
    }

    Component.onDestruction: workbenchBridge.hideTooltip()
}
