#pragma once

#include "gui/AppTheme.h"
#include "workflow/WorkflowGraph.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

#include <algorithm>

// 画布左下角小地图：绘制全局节点分布与当前视口框，点击 / 拖拽可快速定位画布。
// 通过 WorkflowGraph、QGraphicsView 与界面缩放指针读取状态，配色随主题自适应。无信号/槽。
class MiniMapWidget final : public QWidget {
public:
    MiniMapWidget(WorkflowGraph* graph, QGraphicsView* view, double* uiScale, QWidget* parent = nullptr)
        : QWidget(parent), graph_(graph), view_(view), uiScale_(uiScale)
    {
        setObjectName("miniMap");
        setCursor(Qt::PointingHandCursor);
        setToolTip("小地图：点击或拖拽快速定位画布");
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF box = rect().adjusted(1, 1, -1, -1);
        painter.setPen(QPen(QColor(128, 128, 128, 120), 1));
        painter.setBrush(QColor(255, 255, 255, AppTheme::isDarkTheme() ? 36 : 190));
        painter.drawRect(box);

        const QRectF world = worldRect();
        if (!world.isValid() || world.isEmpty()) {
            return;
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(AppTheme::isDarkTheme() ? QColor("#b8b8b8") : QColor("#505050"));
        for (const auto& record : graph_->nodes()) {
            QRectF nodeRect(record.position, QSizeF(AppTheme::nodeMetrics(scale()).width, AppTheme::nodeMetrics(scale()).headerHeight + 78 * scale()));
            painter.drawRect(mapWorldRect(nodeRect, world));
        }

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#202020"), 1.6));
        painter.drawRect(mapWorldRect(viewSceneRect(), world));
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        centerAt(event->position());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (event->buttons().testFlag(Qt::LeftButton)) {
            centerAt(event->position());
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

private:
    double scale() const
    {
        return uiScale_ ? *uiScale_ : 1.0;
    }

    QRectF viewSceneRect() const
    {
        if (!view_) {
            return {};
        }
        return view_->mapToScene(view_->viewport()->rect()).boundingRect();
    }

    QRectF worldRect() const
    {
        QRectF world = viewSceneRect();
        const auto metrics = AppTheme::nodeMetrics(scale());
        for (const auto& record : graph_->nodes()) {
            QRectF nodeRect(record.position, QSizeF(metrics.width, metrics.headerHeight + 78 * scale()));
            world = world.united(nodeRect);
        }
        return world.adjusted(-120 * scale(), -120 * scale(), 120 * scale(), 120 * scale());
    }

    QRectF contentRect() const
    {
        return rect().adjusted(10, 10, -10, -10);
    }

    QPointF mapWorldPoint(const QPointF& point, const QRectF& world) const
    {
        const QRectF content = contentRect();
        const double sx = content.width() / std::max<qreal>(1.0, world.width());
        const double sy = content.height() / std::max<qreal>(1.0, world.height());
        const double s = std::min(sx, sy);
        const QSizeF mappedSize(world.width() * s, world.height() * s);
        const QPointF offset(content.center().x() - mappedSize.width() / 2.0,
                             content.center().y() - mappedSize.height() / 2.0);
        return QPointF(offset.x() + (point.x() - world.left()) * s,
                       offset.y() + (point.y() - world.top()) * s);
    }

    QRectF mapWorldRect(const QRectF& source, const QRectF& world) const
    {
        const QPointF topLeft = mapWorldPoint(source.topLeft(), world);
        const QPointF bottomRight = mapWorldPoint(source.bottomRight(), world);
        return QRectF(topLeft, bottomRight).normalized();
    }

    QPointF unmapWorldPoint(const QPointF& point, const QRectF& world) const
    {
        const QRectF content = contentRect();
        const double sx = content.width() / std::max<qreal>(1.0, world.width());
        const double sy = content.height() / std::max<qreal>(1.0, world.height());
        const double s = std::min(sx, sy);
        const QSizeF mappedSize(world.width() * s, world.height() * s);
        const QPointF offset(content.center().x() - mappedSize.width() / 2.0,
                             content.center().y() - mappedSize.height() / 2.0);
        return QPointF(world.left() + (point.x() - offset.x()) / s,
                       world.top() + (point.y() - offset.y()) / s);
    }

    void centerAt(const QPointF& widgetPosition)
    {
        const QRectF world = worldRect();
        if (!view_ || !world.isValid()) {
            return;
        }
        view_->centerOn(unmapWorldPoint(widgetPosition, world));
        update();
    }

    WorkflowGraph* graph_ = nullptr;
    QGraphicsView* view_ = nullptr;
    double* uiScale_ = nullptr;
};
