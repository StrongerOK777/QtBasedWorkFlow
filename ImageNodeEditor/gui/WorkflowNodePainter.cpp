#include "gui/WorkflowNodePainter.h"

#include "gui/WorkflowNodeDelegate.h"

#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/internal/BasicGraphicsScene.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>

#include <QLinearGradient>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace {

struct CategoryVisual {
    QColor accent;
    QColor stripe;
};

CategoryVisual categoryVisual(const QString& category)
{
    if (category == "输入输出") {
        return {QColor("#4fc1ff"), QColor("#264f78")};
    }
    if (category == "几何变换") {
        return {QColor("#c586c0"), QColor("#5a3f5d")};
    }
    if (category == "色彩处理") {
        return {QColor("#ce9178"), QColor("#704d3f")};
    }
    if (category == "滤波处理") {
        return {QColor("#4ec9b0"), QColor("#315f57")};
    }
    if (category == "合成处理") {
        return {QColor("#dcdcaa"), QColor("#6a6636")};
    }
    return {QColor("#9cdcfe"), QColor("#3c3c3c")};
}

QColor stateAccent(const WorkflowNodeDelegate* delegate, const QColor& normal)
{
    if (!delegate) {
        return normal;
    }
    switch (delegate->executionState()) {
    case NodeExecutionState::Running:
        return QColor("#dcdcaa");
    case NodeExecutionState::Succeeded:
        return QColor("#89d185");
    case NodeExecutionState::Failed:
        return QColor("#f14c4c");
    case NodeExecutionState::CacheHit:
        return QColor("#75beff");
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
    const QColor categoryColor = stateAccent(delegate, visual.accent);
    const QRectF shadowRect = body.translated(0, 4);
    const QRectF borderRect = body.adjusted(0.5, 0.5, -0.5, -0.5);
    const QRectF headerRect(body.left(), body.top(), body.width(), 38.0);
    const bool selected = graphicsNode.isSelected();

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 120));
    painter->drawRect(shadowRect);

    QLinearGradient fill(body.topLeft(), body.bottomLeft());
    fill.setColorAt(0, QColor("#252526"));
    fill.setColorAt(1, QColor("#1f1f1f"));
    painter->setBrush(fill);
    painter->setPen(QPen(selected ? QColor("#3794ff") : QColor("#3c3c3c"), selected ? 2.0 : 1.0));
    painter->drawRect(borderRect);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#2d2d2d"));
    painter->drawRect(headerRect.adjusted(1, 1, -1, 0));
    painter->setBrush(visual.stripe);
    painter->drawRect(QRectF(body.left() + 1, body.top() + 1, 4, std::max<qreal>(1, body.height() - 2)));
    painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 190));
    painter->drawRect(QRectF(body.left() + 1, body.top() + 1, body.width() - 2, 3));
    painter->setPen(QPen(QColor("#3c3c3c"), 1));
    painter->drawLine(QPointF(body.left() + 1, headerRect.bottom()), QPointF(body.right() - 1, headerRect.bottom()));

    if (delegate && delegate->executionState() != NodeExecutionState::NotExecuted) {
        const QRectF strip(body.left() + 8, body.bottom() - 7, body.width() - 16, 3);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 72));
        painter->drawRect(strip);
        if (delegate->executionState() == NodeExecutionState::Running) {
            const qreal segmentWidth = std::max<qreal>(20, strip.width() * 0.22);
            const qreal travel = strip.width() + segmentWidth;
            const qreal offset = std::fmod(delegate->animationPhase() * 8.0, travel) - segmentWidth;
            painter->setBrush(categoryColor);
            painter->drawRect(QRectF(strip.left() + offset, strip.top(), segmentWidth, strip.height()).intersected(strip));
        } else {
            painter->setBrush(categoryColor);
            painter->drawRect(strip);
        }
    }
    if (selected) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(55, 148, 255, 78), 5));
        painter->drawRect(borderRect.adjusted(2, 2, -2, -2));
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
