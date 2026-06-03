#include "gui/WorkflowNodePainter.h"

#include "gui/WorkflowNodeDelegate.h"

#include <QtNodes/DataFlowGraphModel>
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
    const QRectF headerRect(body.left(), body.top(), body.width(), 32.0);
    const bool selected = graphicsNode.isSelected();

    const qreal radius = 12.0;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 柔和阴影
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 90));
    painter->drawRoundedRect(shadowRect, radius, radius);

    // 主体（轻微竖向渐变）+ hairline 边 / 选中柔化蓝
    QLinearGradient fill(body.topLeft(), body.bottomLeft());
    fill.setColorAt(0, QColor("#26282d"));
    fill.setColorAt(1, QColor("#1f2024"));
    painter->setBrush(fill);
    painter->setPen(QPen(selected ? QColor("#6ea0e0") : QColor("#34363b"), selected ? 1.8 : 1.0));
    painter->drawRoundedRect(borderRect, radius, radius);

    // 头部：裁剪到圆角主体内绘制，使上方圆角自然；顶部一抹克制的类别色。
    painter->save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(borderRect, radius, radius);
    painter->setClipPath(clipPath);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#2a2c31"));
    painter->drawRect(QRectF(body.left(), body.top(), body.width(), headerRect.height()));
    painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 150));
    painter->drawRect(QRectF(body.left(), body.top(), body.width(), 3));
    painter->restore();
    painter->setPen(QPen(QColor("#2e2f33"), 1));
    painter->drawLine(QPointF(body.left() + 1, headerRect.bottom()), QPointF(body.right() - 1, headerRect.bottom()));

    if (delegate && delegate->executionState() != NodeExecutionState::NotExecuted) {
        const QRectF strip(body.left() + 10, body.bottom() - 8, body.width() - 20, 3);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 72));
        painter->drawRoundedRect(strip, 1.5, 1.5);
        if (delegate->executionState() == NodeExecutionState::Running) {
            const qreal segmentWidth = std::max<qreal>(20, strip.width() * 0.22);
            const qreal travel = strip.width() + segmentWidth;
            const qreal offset = std::fmod(delegate->animationPhase() * 8.0, travel) - segmentWidth;
            painter->setBrush(categoryColor);
            painter->drawRoundedRect(QRectF(strip.left() + offset, strip.top(), segmentWidth, strip.height()).intersected(strip), 1.5, 1.5);
        } else {
            painter->setBrush(categoryColor);
            painter->drawRoundedRect(strip, 1.5, 1.5);
        }
    }
    if (selected) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(110, 160, 224, 70), 4));
        painter->drawRoundedRect(borderRect.adjusted(2, 2, -2, -2), radius - 2, radius - 2);
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
