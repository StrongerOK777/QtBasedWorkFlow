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

// 不同类别用不同外形（统一大小，靠轮廓+颜色区分）。所有轮廓都画在节点几何矩形内，
// 不改变节点尺寸；端口仍在左右中点。
QPainterPath categoryShapePath(const QString& category, const QRectF& r)
{
    QPainterPath path;
    if (category == "输入输出") {
        // 胶囊：左右两端大圆角
        const qreal rr = std::min<qreal>(r.height() / 2.0, 24.0);
        path.addRoundedRect(r, rr, rr);
    } else if (category == "色彩处理") {
        // 小圆角，更方正
        path.addRoundedRect(r, 5.0, 5.0);
    } else if (category == "滤波处理") {
        // 切角矩形（八边形感）
        const qreal c = std::min<qreal>(14.0, std::min(r.width(), r.height()) * 0.22);
        path.moveTo(r.left() + c, r.top());
        path.lineTo(r.right() - c, r.top());
        path.lineTo(r.right(), r.top() + c);
        path.lineTo(r.right(), r.bottom() - c);
        path.lineTo(r.right() - c, r.bottom());
        path.lineTo(r.left() + c, r.bottom());
        path.lineTo(r.left(), r.bottom() - c);
        path.lineTo(r.left(), r.top() + c);
        path.closeSubpath();
    } else if (category == "合成处理") {
        // 横向六边形：左右两侧斜边收口到端口高度，呼应「合流」
        const qreal c = std::min<qreal>(20.0, r.width() * 0.10);
        const qreal midY = r.center().y();
        path.moveTo(r.left() + c, r.top());
        path.lineTo(r.right() - c, r.top());
        path.lineTo(r.right(), midY);
        path.lineTo(r.right() - c, r.bottom());
        path.lineTo(r.left() + c, r.bottom());
        path.lineTo(r.left(), midY);
        path.closeSubpath();
    } else {
        // 几何变换 / 默认：标准圆角矩形
        path.addRoundedRect(r, 12.0, 12.0);
    }
    return path;
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

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 类别专属外形（统一大小，靠轮廓+颜色区分）
    const QPainterPath shapePath = categoryShapePath(category, borderRect);
    const QPainterPath shadowPath = categoryShapePath(category, shadowRect);

    // 柔和阴影
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 90));
    painter->drawPath(shadowPath);

    // 主体（轻微竖向渐变）+ hairline 边 / 选中柔化蓝
    QLinearGradient fill(body.topLeft(), body.bottomLeft());
    fill.setColorAt(0, QColor("#26282d"));
    fill.setColorAt(1, QColor("#1f2024"));
    painter->setBrush(fill);
    painter->setPen(QPen(selected ? QColor("#6ea0e0") : QColor("#34363b"), selected ? 1.8 : 1.0));
    painter->drawPath(shapePath);

    // 头部：裁剪到外形内绘制，使顶部跟随轮廓；顶部一抹克制的类别色。
    painter->save();
    painter->setClipPath(shapePath);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#2a2c31"));
    painter->drawRect(QRectF(body.left(), body.top(), body.width(), headerRect.height()));
    painter->setBrush(QColor(categoryColor.red(), categoryColor.green(), categoryColor.blue(), 165));
    painter->drawRect(QRectF(body.left(), body.top(), body.width(), 3));
    painter->setPen(QPen(QColor("#2e2f33"), 1));
    painter->drawLine(QPointF(body.left(), headerRect.bottom()), QPointF(body.right(), headerRect.bottom()));
    painter->restore();

    if (delegate && delegate->executionState() != NodeExecutionState::NotExecuted) {
        painter->save();
        painter->setClipPath(shapePath);
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
        painter->restore();
    }
    if (selected) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(110, 160, 224, 70), 4));
        painter->drawPath(shapePath);
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
