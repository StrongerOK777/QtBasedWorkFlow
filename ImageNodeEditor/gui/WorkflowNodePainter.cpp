#include "gui/WorkflowNodePainter.h"

#include "gui/AppTheme.h"
#include "gui/WorkflowNodeDelegate.h"

#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/StyleCollection>
#include <QtNodes/internal/BasicGraphicsScene.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace {

struct CategoryVisual {
    QColor accent;
    enum class Shape {
        Flow,
        CutRect,
        ColorBand,
        Hex,
        Merge,
        Macro
    } shape = Shape::CutRect;
};

CategoryVisual categoryVisual(const QString& category)
{
    if (category == "输入输出") {
        return {QColor("#5a9bd5"), CategoryVisual::Shape::Flow};
    }
    if (category == "几何变换") {
        return {QColor("#8f7ad0"), CategoryVisual::Shape::CutRect};
    }
    if (category == "色彩处理") {
        return {QColor("#d28a55"), CategoryVisual::Shape::ColorBand};
    }
    if (category == "滤波处理") {
        return {QColor("#5aa98b"), CategoryVisual::Shape::Hex};
    }
    if (category == "合成处理") {
        return {QColor("#cf6f8a"), CategoryVisual::Shape::Merge};
    }
    return {QColor("#8ea0aa"), CategoryVisual::Shape::Macro};
}

QPainterPath nodePath(const QRectF& rect, CategoryVisual::Shape shape)
{
    constexpr qreal corner = 14.0;
    QPainterPath path;
    switch (shape) {
    case CategoryVisual::Shape::Flow:
        path.moveTo(rect.left() + 10, rect.top());
        path.lineTo(rect.right() - 18, rect.top());
        path.lineTo(rect.right(), rect.center().y());
        path.lineTo(rect.right() - 18, rect.bottom());
        path.lineTo(rect.left() + 10, rect.bottom());
        path.lineTo(rect.left(), rect.center().y());
        break;
    case CategoryVisual::Shape::CutRect:
    case CategoryVisual::Shape::ColorBand:
        path.moveTo(rect.left() + corner, rect.top());
        path.lineTo(rect.right(), rect.top());
        path.lineTo(rect.right(), rect.bottom() - corner);
        path.lineTo(rect.right() - corner, rect.bottom());
        path.lineTo(rect.left(), rect.bottom());
        path.lineTo(rect.left(), rect.top() + corner);
        break;
    case CategoryVisual::Shape::Hex:
        path.moveTo(rect.left() + corner, rect.top());
        path.lineTo(rect.right() - corner, rect.top());
        path.lineTo(rect.right(), rect.center().y());
        path.lineTo(rect.right() - corner, rect.bottom());
        path.lineTo(rect.left() + corner, rect.bottom());
        path.lineTo(rect.left(), rect.center().y());
        break;
    case CategoryVisual::Shape::Merge:
        path.moveTo(rect.left() + 20, rect.top());
        path.lineTo(rect.right(), rect.top());
        path.lineTo(rect.right() - 10, rect.center().y());
        path.lineTo(rect.right(), rect.bottom());
        path.lineTo(rect.left() + 20, rect.bottom());
        path.lineTo(rect.left(), rect.bottom() - 12);
        path.lineTo(rect.left() + 9, rect.center().y());
        path.lineTo(rect.left(), rect.top() + 12);
        break;
    case CategoryVisual::Shape::Macro:
        path.addRect(rect);
        return path;
    }
    path.closeSubpath();
    return path;
}

QColor stateAccent(const WorkflowNodeDelegate* delegate, const QColor& normal)
{
    if (!delegate) {
        return normal;
    }
    switch (delegate->executionState()) {
    case NodeExecutionState::Running:
        return QColor("#f3d26a");
    case NodeExecutionState::Succeeded:
        return QColor("#d8d8d8");
    case NodeExecutionState::Failed:
        return QColor("#ff453a");
    case NodeExecutionState::CacheHit:
        return QColor("#98a5af");
    case NodeExecutionState::NotExecuted:
        return normal;
    }
    return normal;
}

}

void WorkflowNodePainter::paint(QPainter* painter, QtNodes::NodeGraphicsObject& graphicsNode) const
{
    auto* graph = dynamic_cast<QtNodes::DataFlowGraphModel*>(&graphicsNode.graphModel());
    auto* delegate = graph ? graph->delegateModel<WorkflowNodeDelegate>(graphicsNode.nodeId()) : nullptr;
    const QString category = delegate ? delegate->category() : QString();
    const CategoryVisual visual = categoryVisual(category);
    const QRectF body(QPointF(0, 0), graphicsNode.nodeScene()->nodeGeometry().size(graphicsNode.nodeId()));
    const QPainterPath path = nodePath(body, visual.shape);
    const QColor categoryColor = stateAccent(delegate, visual.accent);
    const bool dark = AppTheme::isDarkTheme();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(dark ? QColor(0, 0, 0, 86) : QColor(32, 32, 32, 36));
    painter->drawPath(nodePath(body.translated(0, 5), visual.shape));

    QLinearGradient fill(body.topLeft(), body.bottomLeft());
    fill.setColorAt(0, dark ? QColor(46, 46, 47, 234) : QColor(255, 255, 255, 242));
    fill.setColorAt(1, dark ? QColor(18, 19, 20, 240) : QColor(224, 224, 224, 238));
    painter->setBrush(fill);
    painter->setPen(QPen(graphicsNode.isSelected() ? categoryColor.lighter(130) : categoryColor,
                         graphicsNode.isSelected() ? 2.6 : 1.6));
    painter->drawPath(path);

    painter->setClipPath(path);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), dark ? 110 : 82));
    painter->drawRect(QRectF(body.left(), body.top(), body.width(), 7));
    if (visual.shape == CategoryVisual::Shape::ColorBand) {
        painter->drawRect(QRectF(body.left(), body.top() + 7, 10, std::max<qreal>(0, body.height() - 7)));
    } else if (visual.shape == CategoryVisual::Shape::Macro) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 150), 1));
        painter->drawRect(body.adjusted(7, 7, -7, -7));
    }

    if (delegate && delegate->executionState() != NodeExecutionState::NotExecuted) {
        const QRectF strip(body.left() + 10, body.bottom() - 7, body.width() - 20, 3);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 92));
        painter->drawRect(strip);
        if (delegate->executionState() == NodeExecutionState::Running) {
            const qreal segmentWidth = std::max<qreal>(20, strip.width() * 0.22);
            const qreal travel = strip.width() + segmentWidth;
            const qreal offset = std::fmod(delegate->animationPhase() * 8.0, travel) - segmentWidth;
            painter->setBrush(categoryColor.lighter(135));
            painter->drawRect(QRectF(strip.left() + offset, strip.top(), segmentWidth, strip.height()).intersected(strip));
        } else {
            painter->setBrush(categoryColor);
            painter->drawRect(strip);
        }
    }
    painter->restore();

    defaultPainter_.drawNodeCaption(painter, graphicsNode);
    defaultPainter_.drawNodeLabel(painter, graphicsNode);
    defaultPainter_.drawEntryLabels(painter, graphicsNode);
    defaultPainter_.drawConnectionPoints(painter, graphicsNode);
    defaultPainter_.drawFilledConnectionPoints(painter, graphicsNode);
    defaultPainter_.drawValidationIcon(painter, graphicsNode);
    defaultPainter_.drawProcessingIndicator(painter, graphicsNode);
    defaultPainter_.drawProgressValue(painter, graphicsNode);
}
