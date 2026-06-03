import QtQuick

Item {
    id: icon
    required property string name
    property color strokeColor: "#cccccc"
    property color fillColor: "transparent"
    property real strokeWidth: 1.7

    width: 24
    height: 24

    FontLoader {
        id: codicons
        source: "qrc:/codicons/codicon.ttf"
    }

    function glyph(iconName) {
        var map = {
            "add": 0xea60,
            "plus": 0xea60,
            "warning": 0xea6c,
            "problems": 0xea6c,
            "search": 0xea6d,
            "more": 0xea7c,
            "command": 0xea7c,
            "organization": 0xea7e,
            "terminal": 0xea85,
            "error": 0xea87,
            "arrow-left": 0xea9b,
            "arrow-right": 0xea9c,
            "back": 0xeab5,
            "chevron-left": 0xeab5,
            "forward": 0xeab6,
            "chevron-right": 0xeab6,
            "window-close": 0xeab8,
            "chrome-close": 0xeab8,
            "window-maximize": 0xeab9,
            "chrome-maximize": 0xeab9,
            "window-minimize": 0xeaba,
            "chrome-minimize": 0xeaba,
            "window-restore": 0xeabb,
            "chrome-restore": 0xeabb,
            "debug-start": 0xead3,
            "files": 0xeaf0,
            "filter": 0xeaf1,
            "settings": 0xeaf8,
            "gear": 0xeaf8,
            "git-merge": 0xeafe,
            "source-control": 0xeafe,
            "history": 0xeafe,
            "workflow": 0xeb03,
            "graph": 0xeb03,
            "run": 0xeb2c,
            "play": 0xeb2c,
            "preview": 0xeb2f,
            "settings-gear": 0xeb51,
            "symbol-color": 0xeb5c,
            "symbol-misc": 0xeb63,
            "nodes": 0xeb63,
            "templates": 0xeb63,
            "extensions": 0xeb63,
            "list-tree": 0xeb86,
            "run-diagnostics": 0xeb91,
            "debug-alt": 0xeb91,
            "layout": 0xebeb,
            "panel": 0xebf2,
            "layout-panel": 0xebf2,
            "layout-sidebar-right": 0xebf4,
            "app": 0xeb03
        }
        return String.fromCharCode(map[iconName] || 0xeb63)
    }

    Text {
        anchors.centerIn: parent
        text: icon.glyph(icon.name)
        color: icon.strokeColor
        font.family: codicons.name
        font.pixelSize: Math.min(icon.width, icon.height)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        renderType: Text.NativeRendering
    }
}
