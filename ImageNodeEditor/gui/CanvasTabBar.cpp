#include "gui/CanvasTabBar.h"

#include "gui/AppTheme.h"

#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>

CanvasTabBar::CanvasTabBar(QWidget* parent) : QTabBar(parent)
{
    setDrawBase(false);
    setExpanding(false);
    setMovable(true);
    setMouseTracking(true);
}

QSize CanvasTabBar::tabSizeHint(int index) const
{
    const QSize base = QTabBar::tabSizeHint(index);
    const QFontMetrics fm = fontMetrics();
    // 文本 + 左右内边距 + close 按钮预留宽度；高度按当前（已缩放）字体走，给更舒展的留白。
    const int width = fm.horizontalAdvance(tabText(index)) + 64;
    const int height = fm.height() + 24;
    return QSize(std::max(width, 150), std::max(base.height(), height));
}

void CanvasTabBar::setHoverIndex(int index)
{
    if (hoverIndex_ == index) {
        return;
    }
    hoverIndex_ = index;
    update();
}

void CanvasTabBar::mouseMoveEvent(QMouseEvent* event)
{
    setHoverIndex(tabAt(event->pos()));
    QTabBar::mouseMoveEvent(event);
}

void CanvasTabBar::leaveEvent(QEvent* event)
{
    setHoverIndex(-1);
    QTabBar::leaveEvent(event);
}

void CanvasTabBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    const AppTheme::Palette p = AppTheme::palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 标签条底色用 panel，使激活标签（base 内容色）凸显并与下方画布融合。
    painter.fillRect(rect(), p.panel);
    // 整条底部 hairline；激活标签会盖住其下方的这段线，从而“连入”内容区。
    painter.setPen(QPen(p.hairline, 1));
    painter.drawLine(QPointF(rect().left(), rect().bottom() + 0.5),
                     QPointF(rect().right(), rect().bottom() + 0.5));

    const int current = currentIndex();

    // 标签文字：略大一点、清晰；激活标签用半粗体强调。
    QFont labelFont = font();
    labelFont.setPointSizeF(labelFont.pointSizeF() + 0.5);

    auto drawInactive = [&](int index) {
        const QRect r = tabRect(index);
        const bool hovered = index == hoverIndex_;
        if (hovered) {
            QRectF rf(r);
            rf.adjust(4.0, 6.0, -4.0, 0.0);
            QPainterPath path;
            path.addRoundedRect(rf.adjusted(0, 0, 0, 12), 9, 9);  // 圆角顶、底部延伸到条外被裁掉
            painter.setPen(Qt::NoPen);
            painter.setBrush(p.elevated);
            painter.setClipRect(r.adjusted(0, 0, 0, 1));
            painter.drawPath(path);
            painter.setClipping(false);
        }
        // 相邻非激活标签之间画一条细分隔线（hover/相邻激活时不画）。
        if (!hovered && index != current && index + 1 < count() && index + 1 != current
            && (index - 1 < 0 || index - 1 != current)) {
            const int sx = r.right();
            painter.setPen(QPen(p.hairline, 1));
            painter.drawLine(QPointF(sx + 0.5, r.top() + r.height() * 0.30),
                             QPointF(sx + 0.5, r.bottom() - r.height() * 0.26));
        }
        const QRect textRect = r.adjusted(16, 2, -28, 0);
        labelFont.setWeight(QFont::Normal);
        painter.setFont(labelFont);
        painter.setPen(p.textSecondary);
        const QString label = QFontMetrics(labelFont).elidedText(tabText(index), Qt::ElideRight, textRect.width());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    };

    auto drawActive = [&](int index) {
        const QRect r = tabRect(index);
        const qreal topR = 12.0;   // 顶部圆角
        const qreal flare = 11.0;  // 底部外扩弧
        const qreal l = r.left() + 0.5;
        const qreal rt = r.right() - 0.5;
        const qreal top = r.top() + 6.0;
        const qreal bot = r.bottom() + 1.0;  // 一直画到条底，盖住底部 hairline

        QPainterPath path;
        path.moveTo(l - flare, bot);
        path.quadTo(l, bot, l, bot - flare);
        path.lineTo(l, top + topR);
        path.quadTo(l, top, l + topR, top);
        path.lineTo(rt - topR, top);
        path.quadTo(rt, top, rt, top + topR);
        path.lineTo(rt, bot - flare);
        path.quadTo(rt, bot, rt + flare, bot);
        path.closeSubpath();

        painter.setPen(Qt::NoPen);
        painter.setBrush(p.base);  // 与下方画布内容同色，融为一体
        painter.drawPath(path);
        // 顶部与两侧 hairline 描边（底部不描，保持与内容连通）。
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(p.hairline, 1));
        QPainterPath stroke;
        stroke.moveTo(l - flare, bot);
        stroke.quadTo(l, bot, l, bot - flare);
        stroke.lineTo(l, top + topR);
        stroke.quadTo(l, top, l + topR, top);
        stroke.lineTo(rt - topR, top);
        stroke.quadTo(rt, top, rt, top + topR);
        stroke.lineTo(rt, bot - flare);
        stroke.quadTo(rt, bot, rt + flare, bot);
        painter.drawPath(stroke);

        const QRect textRect = r.adjusted(16, 2, -28, 0);
        labelFont.setWeight(QFont::DemiBold);
        painter.setFont(labelFont);
        painter.setPen(p.textPrimary);
        const QString label = QFontMetrics(labelFont).elidedText(tabText(index), Qt::ElideRight, textRect.width());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    };

    for (int i = 0; i < count(); ++i) {
        if (i != current) {
            drawInactive(i);
        }
    }
    if (current >= 0 && current < count()) {
        drawActive(current);
    }
}
