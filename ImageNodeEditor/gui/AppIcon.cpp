#include "gui/AppIcon.h"

#include <QPainter>
#include <QPixmap>

namespace {

QPixmap renderIcon(int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal s = size / 64.0;
    auto rect = [s](qreal x, qreal y, qreal w, qreal h) {
        return QRectF(x * s, y * s, w * s, h * s);
    };
    auto point = [s](qreal x, qreal y) {
        return QPointF(x * s, y * s);
    };

    painter.setPen(QPen(QColor("#3794ff"), 2.2 * s));
    painter.setBrush(QColor("#1e1e1e"));
    painter.drawRect(rect(8, 8, 48, 48));

    painter.setPen(QPen(QColor("#75beff"), 2.0 * s, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter.drawLine(point(22, 23), point(42, 23));
    painter.drawLine(point(22, 41), point(42, 41));
    painter.drawLine(point(22, 23), point(22, 41));
    painter.drawLine(point(42, 23), point(42, 41));
    painter.drawLine(point(22, 32), point(42, 32));

    painter.setPen(QPen(QColor("#cccccc"), 1.5 * s));
    painter.setBrush(QColor("#252526"));
    painter.drawRect(rect(14, 16, 16, 14));
    painter.drawRect(rect(34, 34, 16, 14));

    painter.setBrush(QColor("#3794ff"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect(18, 20, 4, 4));
    painter.drawRect(rect(24, 20, 4, 4));
    painter.drawRect(rect(38, 38, 4, 4));
    painter.drawRect(rect(44, 38, 4, 4));

    painter.setPen(QPen(QColor("#cccccc"), 2.0 * s));
    painter.setBrush(QColor("#1e1e1e"));
    painter.drawRect(rect(19, 29, 6, 6));
    painter.drawRect(rect(39, 29, 6, 6));
    painter.drawRect(rect(29, 39, 6, 6));
    painter.drawLine(point(25, 32), point(39, 32));
    painter.drawLine(point(42, 35), point(32, 39));

    return pixmap;
}

}

namespace AppIcon {

QIcon makeAppIcon()
{
    QIcon icon;
    for (int size : {16, 24, 32, 48, 64, 128}) {
        icon.addPixmap(renderIcon(size));
    }
    return icon;
}

}
