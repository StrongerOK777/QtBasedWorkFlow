import QtQuick

Canvas {
    id: icon
    required property string name
    property color strokeColor: "#cccccc"
    property color fillColor: "transparent"
    property real strokeWidth: 1.7

    width: 24
    height: 24

    onNameChanged: requestPaint()
    onStrokeColorChanged: requestPaint()
    onFillColorChanged: requestPaint()
    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        ctx.clearRect(0, 0, width, height)
        ctx.lineWidth = strokeWidth
        ctx.strokeStyle = strokeColor
        ctx.fillStyle = fillColor
        ctx.lineCap = "square"
        ctx.lineJoin = "miter"

        function line(x1, y1, x2, y2) {
            ctx.beginPath()
            ctx.moveTo(x1, y1)
            ctx.lineTo(x2, y2)
            ctx.stroke()
        }
        function rect(x, y, w, h, fill) {
            ctx.beginPath()
            ctx.rect(x, y, w, h)
            fill ? ctx.fill() : ctx.stroke()
        }
        function circle(x, y, r) {
            ctx.beginPath()
            ctx.arc(x, y, r, 0, Math.PI * 2)
            ctx.stroke()
        }

        if (name === "nodes") {
            rect(4, 4, 7, 7, false)
            rect(13, 13, 7, 7, false)
            line(11, 7.5, 15, 13)
            return
        }
        if (name === "workflow") {
            rect(4, 5, 6, 6, false)
            rect(14, 5, 6, 6, false)
            rect(9, 15, 6, 5, false)
            line(10, 8, 14, 8)
            line(17, 11, 12, 15)
            line(7, 11, 12, 15)
            return
        }
        if (name === "search") {
            circle(10, 10, 5.5)
            line(14, 14, 20, 20)
            return
        }
        if (name === "command") {
            line(5, 7, 11, 12)
            line(11, 12, 5, 17)
            line(13, 17, 20, 17)
            return
        }
        if (name === "preview") {
            rect(4, 5, 16, 12, false)
            line(8, 20, 16, 20)
            line(12, 17, 12, 20)
            return
        }
        if (name === "files") {
            rect(6, 4, 10, 13, false)
            line(9, 4, 9, 7)
            rect(9, 7, 10, 13, false)
            return
        }
        if (name === "symbol-color") {
            rect(5, 5, 14, 14, false)
            line(5, 10, 19, 10)
            line(10, 5, 10, 19)
            return
        }
        if (name === "filter") {
            line(5, 6, 19, 6)
            line(7, 10, 17, 10)
            line(9, 14, 15, 14)
            line(11, 18, 13, 18)
            return
        }
        if (name === "git-merge") {
            circle(7, 6, 2.5)
            circle(17, 18, 2.5)
            circle(7, 18, 2.5)
            line(7, 8.5, 7, 15.5)
            line(7, 9, 17, 15.5)
            return
        }
        if (name === "symbol-misc") {
            rect(5, 5, 6, 6, false)
            rect(13, 5, 6, 6, false)
            rect(5, 13, 6, 6, false)
            rect(13, 13, 6, 6, false)
            return
        }
        if (name === "back") {
            line(15, 6, 9, 12)
            line(9, 12, 15, 18)
            return
        }
        if (name === "forward") {
            line(9, 6, 15, 12)
            line(15, 12, 9, 18)
            return
        }
        if (name === "add") {
            line(12, 5, 12, 19)
            line(5, 12, 19, 12)
            return
        }
        if (name === "layout") {
            rect(4, 5, 6, 5, false)
            rect(14, 5, 6, 5, false)
            rect(9, 15, 6, 5, false)
            line(7, 10, 7, 13)
            line(17, 10, 17, 13)
            line(7, 13, 17, 13)
            line(12, 13, 12, 15)
            return
        }
        if (name === "panel") {
            rect(4, 5, 16, 14, false)
            line(4, 14, 20, 14)
            return
        }
        if (name === "run") {
            ctx.beginPath()
            ctx.moveTo(8, 5)
            ctx.lineTo(18, 12)
            ctx.lineTo(8, 19)
            ctx.closePath()
            ctx.fillStyle = strokeColor
            ctx.fill()
            return
        }
        rect(5, 5, 14, 14, false)
    }
}
