#include "gui/AppIcon.h"

#include "gui/AppTheme.h"

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>

#include <cmath>

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

QIcon lineIcon(const QString& name)
{
    constexpr int size = 64;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    const QColor ink = AppTheme::isDarkTheme() ? QColor("#e0e0e0") : QColor("#303030");
    const QColor accent = AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#505050");
    const QColor warm = AppTheme::isDarkTheme() ? QColor("#b8b8b8") : QColor("#707070");
    QPen pen(ink, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    auto page = [&] {
        painter.drawRect(QRectF(18, 10, 28, 42));
        painter.drawLine(QPointF(34, 10), QPointF(46, 22));
        painter.drawLine(QPointF(34, 10), QPointF(34, 22));
        painter.drawLine(QPointF(34, 22), QPointF(46, 22));
    };
    auto folder = [&] {
        painter.drawPath([] {
            QPainterPath path;
            path.moveTo(9, 23);
            path.lineTo(25, 23);
            path.lineTo(30, 17);
            path.lineTo(42, 17);
            path.quadTo(48, 17, 50, 23);
            path.lineTo(55, 23);
            path.lineTo(50, 50);
            path.lineTo(12, 50);
            path.closeSubpath();
            return path;
        }());
    };

    if (name == "new") {
        page();
        painter.setPen(QPen(accent, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(32, 29), QPointF(32, 43));
        painter.drawLine(QPointF(25, 36), QPointF(39, 36));
    } else if (name == "plus") {
        // 简洁的「+」，用于画布标签条的新建按钮，风格与标签更一致。
        painter.setPen(QPen(ink, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(32, 20), QPointF(32, 44));
        painter.drawLine(QPointF(20, 32), QPointF(44, 32));
    } else if (name == "open") {
        folder();
        painter.setPen(QPen(accent, 4.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 43), QPointF(32, 28));
        painter.drawLine(QPointF(24, 35), QPointF(32, 27));
        painter.drawLine(QPointF(40, 35), QPointF(32, 27));
    } else if (name == "save") {
        painter.drawRect(QRectF(13, 11, 38, 42));
        painter.drawRect(QRectF(21, 11, 22, 14));
        painter.drawRect(QRectF(22, 35, 20, 14));
    } else if (name == "saveAs") {
        painter.drawRect(QRectF(12, 12, 34, 40));
        painter.drawRect(QRectF(20, 12, 18, 12));
        painter.setPen(QPen(warm, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(40, 41), QPointF(54, 27));
        painter.drawLine(QPointF(47, 27), QPointF(54, 27));
        painter.drawLine(QPointF(54, 27), QPointF(54, 34));
    } else if (name == "run") {
        painter.setBrush(QColor("#34c759"));
        painter.setPen(Qt::NoPen);
        QPainterPath play;
        play.moveTo(23, 14);
        play.lineTo(50, 32);
        play.lineTo(23, 50);
        play.closeSubpath();
        painter.drawPath(play);
    } else if (name == "exportCanvas") {
        painter.drawRect(QRectF(12, 14, 40, 32));
        painter.drawLine(QPointF(20, 24), QPointF(44, 24));
        painter.drawLine(QPointF(20, 32), QPointF(36, 32));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 49), QPointF(32, 32));
        painter.drawLine(QPointF(24, 41), QPointF(32, 49));
        painter.drawLine(QPointF(40, 41), QPointF(32, 49));
    } else if (name == "exportWorkflow") {
        page();
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 48), QPointF(32, 29));
        painter.drawLine(QPointF(24, 40), QPointF(32, 48));
        painter.drawLine(QPointF(40, 40), QPointF(32, 48));
    } else if (name == "exportPreview") {
        painter.drawRect(QRectF(12, 14, 40, 30));
        painter.drawLine(QPointF(18, 38), QPointF(28, 27));
        painter.drawLine(QPointF(28, 27), QPointF(35, 34));
        painter.drawLine(QPointF(35, 34), QPointF(44, 23));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 51), QPointF(32, 34));
        painter.drawLine(QPointF(24, 43), QPointF(32, 51));
        painter.drawLine(QPointF(40, 43), QPointF(32, 51));
    } else if (name == "zoomIn" || name == "zoomOut") {
        painter.drawEllipse(QPointF(28, 28), 16, 16);
        painter.drawLine(QPointF(40, 40), QPointF(52, 52));
        painter.setPen(QPen(accent, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(20, 28), QPointF(36, 28));
        if (name == "zoomIn") {
            painter.drawLine(QPointF(28, 20), QPointF(28, 36));
        }
    } else if (name == "scaleReset") {
        painter.drawArc(QRectF(16, 16, 32, 32), 45 * 16, 285 * 16);
        painter.drawLine(QPointF(45, 15), QPointF(45, 27));
        painter.drawLine(QPointF(45, 15), QPointF(33, 15));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(QRectF(18, 24, 28, 18), Qt::AlignCenter, "1:1");
    } else if (name == "dockLeft") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.fillRect(QRectF(12, 12, 13, 40), AppTheme::isDarkTheme() ? QColor(238, 238, 238, 62) : QColor(80, 80, 80, 70));
        painter.drawLine(QPointF(25, 12), QPointF(25, 52));
    } else if (name == "dockRight") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.fillRect(QRectF(39, 12, 13, 40), AppTheme::isDarkTheme() ? QColor(238, 238, 238, 62) : QColor(80, 80, 80, 70));
        painter.drawLine(QPointF(39, 12), QPointF(39, 52));
    } else if (name == "dockBottom") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.fillRect(QRectF(12, 39, 40, 13), AppTheme::isDarkTheme() ? QColor(238, 238, 238, 62) : QColor(80, 80, 80, 70));
        painter.drawLine(QPointF(12, 39), QPointF(52, 39));
    } else if (name == "nodes") {
        painter.drawRect(QRectF(11, 12, 17, 15));
        painter.drawRect(QRectF(36, 12, 17, 15));
        painter.drawRect(QRectF(23, 38, 18, 15));
        painter.drawLine(QPointF(20, 27), QPointF(29, 38));
        painter.drawLine(QPointF(45, 27), QPointF(35, 38));
    } else if (name == "workflow") {
        painter.drawRect(QRectF(13, 12, 16, 13));
        painter.drawRect(QRectF(35, 26, 16, 13));
        painter.drawRect(QRectF(13, 41, 16, 13));
        painter.drawLine(QPointF(29, 18), QPointF(35, 31));
        painter.drawLine(QPointF(35, 35), QPointF(29, 47));
    } else if (name == "layoutReset") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.drawLine(QPointF(25, 12), QPointF(25, 52));
        painter.drawLine(QPointF(12, 39), QPointF(52, 39));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawArc(QRectF(22, 22, 20, 20), 40 * 16, 260 * 16);
    } else if (name == "fullscreen") {
        painter.drawLine(QPointF(14, 26), QPointF(14, 14));
        painter.drawLine(QPointF(14, 14), QPointF(26, 14));
        painter.drawLine(QPointF(38, 14), QPointF(50, 14));
        painter.drawLine(QPointF(50, 14), QPointF(50, 26));
        painter.drawLine(QPointF(50, 38), QPointF(50, 50));
        painter.drawLine(QPointF(50, 50), QPointF(38, 50));
        painter.drawLine(QPointF(26, 50), QPointF(14, 50));
        painter.drawLine(QPointF(14, 50), QPointF(14, 38));
    } else if (name == "settings") {
        painter.drawEllipse(QPointF(32, 32), 8, 8);
        constexpr double pi = 3.14159265358979323846;
        for (int i = 0; i < 8; ++i) {
            const double a = i * pi / 4.0;
            const QPointF inner(32 + std::cos(a) * 16, 32 + std::sin(a) * 16);
            const QPointF outer(32 + std::cos(a) * 23, 32 + std::sin(a) * 23);
            painter.drawLine(inner, outer);
        }
    } else if (name == "clearLog") {
        painter.drawRect(QRectF(16, 14, 32, 38));
        painter.drawLine(QPointF(22, 24), QPointF(42, 24));
        painter.drawLine(QPointF(22, 32), QPointF(38, 32));
        painter.drawLine(QPointF(22, 40), QPointF(34, 40));
        painter.setPen(QPen(warm, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(44, 14), QPointF(54, 24));
        painter.drawLine(QPointF(54, 14), QPointF(44, 24));
    } else if (name == "back" || name == "forward") {
        const qreal start = name == "back" ? 42 : 22;
        const qreal tip = name == "back" ? 20 : 44;
        painter.drawLine(QPointF(start, 18), QPointF(tip, 32));
        painter.drawLine(QPointF(tip, 32), QPointF(start, 46));
    } else if (name == "more") {
        painter.setBrush(ink);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(20, 32), 4.2, 4.2);
        painter.drawEllipse(QPointF(32, 32), 4.2, 4.2);
        painter.drawEllipse(QPointF(44, 32), 4.2, 4.2);
    }
    return QIcon(pix);
}

}
