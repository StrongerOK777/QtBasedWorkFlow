#include "gui/WorkflowCanvas.h"

#include "gui/AppTheme.h"
#include "gui/WorkflowNodeDelegate.h"
#include "gui/WorkflowNodePainter.h"
#include "nodes/ImageNode.h"
#include "nodes/NodeFactory.h"
#include "workflow/WorkflowGraph.h"

#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/Definitions>
#include <QtNodes/GraphicsView>
#include <QtNodes/ConnectionStyle>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/NodeStyle>
#include <QtNodes/internal/AbstractConnectionPainter.hpp>
#include <QtNodes/internal/AbstractGraphModel.hpp>
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/ConnectionState.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QtNodes/internal/StyleCollection.hpp>

#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QImageReader>
#include <QKeyEvent>
#include <QLineF>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QTimer>
#include <QToolTip>
#include <QUrl>
#include <QVariant>
#include <QWheelEvent>

#include <cmath>
#include <memory>

namespace {

bool samePoint(const QPointF& first, const QPointF& second)
{
    return qFuzzyCompare(first.x() + 1.0, second.x() + 1.0) &&
           qFuzzyCompare(first.y() + 1.0, second.y() + 1.0);
}

int portIndex(const QSharedPointer<ImageNode>& node, const QString& portName, PortDirection direction)
{
    if (!node) {
        return -1;
    }
    const QVector<PortInfo> ports = direction == PortDirection::Input ? node->inputPorts() : node->outputPorts();
    for (int index = 0; index < ports.size(); ++index) {
        if (ports.at(index).name == portName) {
            return index;
        }
    }
    return -1;
}

QString portName(const QSharedPointer<ImageNode>& node, int index, PortDirection direction)
{
    if (!node) {
        return {};
    }
    const QVector<PortInfo> ports = direction == PortDirection::Input ? node->inputPorts() : node->outputPorts();
    return index >= 0 && index < ports.size() ? ports.at(index).name : QString();
}

QString categoryForType(const QString& typeName)
{
    for (const auto& descriptor : NodeFactory::instance().descriptors()) {
        if (descriptor.typeName == typeName) {
            return descriptor.category;
        }
    }
    return QStringLiteral("高级功能");
}

bool isKnownNodeType(const QString& typeName)
{
    for (const auto& descriptor : NodeFactory::instance().descriptors()) {
        if (descriptor.typeName == typeName) {
            return true;
        }
    }
    return false;
}

// 拖入的外部文件中筛选出可接受的本地文件：图片（按 Qt 支持的格式）或 workflow JSON。
QStringList droppableLocalFiles(const QMimeData* mime)
{
    QStringList files;
    if (!mime || !mime->hasUrls()) {
        return files;
    }
    static const QStringList imageSuffixes = [] {
        QStringList suffixes;
        for (const QByteArray& format : QImageReader::supportedImageFormats()) {
            suffixes.append(QString::fromLatin1(format).toLower());
        }
        return suffixes;
    }();
    for (const QUrl& url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        const QString suffix = QFileInfo(path).suffix().toLower();
        if (suffix == "json" || imageSuffixes.contains(suffix)) {
            files.append(path);
        }
    }
    return files;
}

// 自定义连线绘制：按数据类型着色（颜色来自 AppTheme，深浅主题自适应）。
// 行为与 QtNodes DefaultConnectionPainter 一致：悬停/选中光晕、拖拽虚线、普通曲线、端点圆点。
// 放在应用层而不是 patch third_party，避免原生 VS 工程重编预编译 QtNodes.lib。
class WorkflowConnectionPainter final : public QtNodes::AbstractConnectionPainter {
public:
    void paint(QPainter* painter, const QtNodes::ConnectionGraphicsObject& cgo) const override
    {
        const auto& style = QtNodes::StyleCollection::connectionStyle();
        const QPainterPath cubic = cubicPath(cgo);
        const bool requiresPort = cgo.connectionState().requiresPort();

        QColor typeColor = style.normalColor();
        if (!requiresPort) {
            const auto cId = cgo.connectionId();
            const auto dataType = cgo.graphModel()
                                      .portData(cId.outNodeId, QtNodes::PortType::Out, cId.outPortIndex,
                                                QtNodes::PortRole::DataType)
                                      .value<QtNodes::NodeDataType>();
            typeColor = AppTheme::portTypeColor(dataType.id);
        }

        // 悬停 / 选中光晕
        if (cgo.connectionState().hovered() || cgo.isSelected()) {
            QPen halo;
            halo.setWidth(int(2 * style.lineWidth()));
            halo.setColor(cgo.isSelected() ? style.selectedHaloColor() : style.hoveredColor());
            painter->setPen(halo);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(cubic);
        }

        // 正在拖拽的未完成连线：虚线
        if (requiresPort || cgo.connectionState().frozen()) {
            QPen pen;
            pen.setWidth(int(style.constructionLineWidth()));
            pen.setColor(style.constructionColor());
            pen.setStyle(Qt::DashLine);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(cubic);
        } else {
            QPen pen;
            pen.setWidthF(style.lineWidth());
            pen.setColor(cgo.isSelected() ? style.selectedColor() : typeColor);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(cubic);
        }

        // 两端小圆点
        const double pointRadius = style.pointDiameter() / 2.0;
        painter->setPen(typeColor);
        painter->setBrush(typeColor);
        painter->drawEllipse(cgo.out(), pointRadius, pointRadius);
        painter->drawEllipse(cgo.in(), pointRadius, pointRadius);
    }

    QPainterPath getPainterStroke(const QtNodes::ConnectionGraphicsObject& cgo) const override
    {
        const QPainterPath cubic = cubicPath(cgo);
        QPainterPath result(cgo.endPoint(QtNodes::PortType::Out));
        constexpr unsigned int segments = 20;
        for (unsigned int i = 0; i < segments; ++i) {
            const double ratio = double(i + 1) / segments;
            result.lineTo(cubic.pointAtPercent(ratio));
        }
        QPainterPathStroker stroker;
        stroker.setWidth(10.0);
        return stroker.createStroke(result);
    }

private:
    static QPainterPath cubicPath(const QtNodes::ConnectionGraphicsObject& connection)
    {
        const QPointF& in = connection.endPoint(QtNodes::PortType::In);
        const QPointF& out = connection.endPoint(QtNodes::PortType::Out);
        const auto c1c2 = connection.pointsC1C2();
        QPainterPath cubic(out);
        cubic.cubicTo(c1c2.first, c1c2.second, in);
        return cubic;
    }
};

// 按当前主题 palette 生成 Qt Nodes 全局样式。无 static 守卫，主题切换时 rebuild()
// 会重新调用，使节点边框 / 渐变 / 连线 / 端口颜色跟随深色 / 浅色切换。
void applyQtNodesStyle()
{
    const AppTheme::Palette p = AppTheme::palette();
    auto arr = [](const QColor& c) {
        return QString("[%1, %2, %3]").arg(c.red()).arg(c.green()).arg(c.blue());
    };
    QtNodes::NodeStyle::setNodeStyle(QString(R"({
        "NodeStyle": {
            "NormalBoundaryColor": %1,
            "SelectedBoundaryColor": %2,
            "GradientColor0": %3,
            "GradientColor1": %3,
            "GradientColor2": %4,
            "GradientColor3": %4,
            "ShadowColor": [0, 0, 0],
            "ShadowEnabled": false,
            "FontColor": %5,
            "FontColorFaded": %6,
            "ConnectionPointColor": %7,
            "FilledConnectionPointColor": %8,
            "ErrorColor": %9,
            "WarningColor": %10,
            "ToolTipIconColor": %5,
            "PenWidth": 1.0,
            "HoveredPenWidth": 1.6,
            "ConnectionPointDiameter": 9.0,
            "Opacity": 1.0
        }
    })")
                                         .arg(arr(p.nodeBorder), arr(p.nodeSelected), arr(p.nodeTop), arr(p.nodeBottom),
                                              arr(p.nodeText), arr(p.nodeTextFaded), arr(p.inputPort), arr(p.outputPort),
                                              arr(p.danger))
                                         .arg(arr(p.warning)));
    QtNodes::ConnectionStyle::setConnectionStyle(QString(R"({
        "ConnectionStyle": {
            "ConstructionColor": %1,
            "NormalColor": %2,
            "SelectedColor": %3,
            "SelectedHaloColor": %3,
            "HoveredColor": %4,
            "LineWidth": 2.0,
            "ConstructionLineWidth": 2.0,
            "PointDiameter": 9.0,
            "UseDataDefinedColors": false
        }
    })")
                                                     .arg(arr(p.pendingEdge), arr(p.edge), arr(p.edgeSelected),
                                                          arr(p.accentHover)));
}

void paintCanvasGrid(QPainter* painter, const QRectF& rect, double uiScale)
{
    const auto colors = AppTheme::colors();
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setColorAt(0, colors.canvasTop);
    gradient.setColorAt(1, colors.canvasBottom);
    painter->fillRect(rect, gradient);

    const qreal step = 30.0 * uiScale;
    const qreal dotRadius = std::max<qreal>(0.9, 1.0 * uiScale);
    const qreal left = std::floor(rect.left() / step) * step;
    const qreal top = std::floor(rect.top() / step) * step;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(colors.canvasDot);
    for (qreal x = left; x < rect.right(); x += step) {
        for (qreal y = top; y < rect.bottom(); y += step) {
            painter->drawEllipse(QPointF(x, y), dotRadius, dotRadius);
        }
    }
}

class WorkflowGraphicsView final : public QtNodes::GraphicsView {
public:
    WorkflowGraphicsView(QtNodes::BasicGraphicsScene* scene,
                         double* uiScale,
                         std::function<void()> deleteSelection,
                         std::function<void()> copySelection,
                         std::function<void(const QPointF&)> quickPalette,
                         std::function<void(const QString&, const QPointF&)> nodeDropped,
                         std::function<void(double)> wheelZoomRequested,
                         std::function<void(const QStringList&, const QPointF&)> filesDropped,
                         QWidget* parent)
        : QtNodes::GraphicsView(scene, parent),
          uiScale_(uiScale),
          deleteSelection_(std::move(deleteSelection)),
          copySelection_(std::move(copySelection)),
          quickPalette_(std::move(quickPalette)),
          nodeDropped_(std::move(nodeDropped)),
          wheelZoomRequested_(std::move(wheelZoomRequested)),
          filesDropped_(std::move(filesDropped))
    {
        setAcceptDrops(true);
        setMouseTracking(true);  // 端口悬停提示需要无按键的 mouseMoveEvent
    }

    // ---- 拖动节点时的对齐辅助线 ----
    void setAlignmentGuides(QVector<QLineF> guides)
    {
        if (guides == alignmentGuides_) {
            return;
        }
        alignmentGuides_ = std::move(guides);
        if (viewport()) {
            viewport()->update();
        }
    }

    void onDeleteSelectedObjects() override
    {
        if (deleteSelection_) {
            deleteSelection_();
        }
    }

    void onCopySelectedObjects() override
    {
        if (copySelection_) {
            copySelection_();
        }
    }

    void clearTransientItems()
    {
        setAlignmentGuides({});
    }

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override
    {
        paintCanvasGrid(painter, rect, uiScale_ ? *uiScale_ : 1.0);
    }

    void drawForeground(QPainter* painter, const QRectF& rect) override
    {
        QtNodes::GraphicsView::drawForeground(painter, rect);
        if (alignmentGuides_.isEmpty()) {
            return;
        }
        painter->save();
        QPen pen(AppTheme::palette().accent, 0);  // 宽度 0 = cosmetic，缩放下保持 1px
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        for (const QLineF& guide : alignmentGuides_) {
            painter->drawLine(guide);
        }
        painter->restore();
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        QtNodes::GraphicsView::mouseReleaseEvent(event);
        setAlignmentGuides({});
    }

    // 端口悬停提示：鼠标靠近端口圆点时显示「方向 + 端口名 + 数据类型」。
    void mouseMoveEvent(QMouseEvent* event) override
    {
        QtNodes::GraphicsView::mouseMoveEvent(event);
        if (event->buttons() != Qt::NoButton || !nodeScene()) {
            return;
        }
        const QPointF scenePos = mapToScene(event->position().toPoint());
        auto& graphModel = nodeScene()->graphModel();
        auto& geometry = nodeScene()->nodeGeometry();
        const double hitRadius = QtNodes::StyleCollection::connectionStyle().pointDiameter() / 2.0 + 4.0;

        QString tooltip;
        const QList<QGraphicsItem*> hits = items(event->position().toPoint());
        for (QGraphicsItem* item : hits) {
            auto* nodeObject = dynamic_cast<QtNodes::NodeGraphicsObject*>(item);
            if (!nodeObject) {
                continue;
            }
            const auto nodeId = nodeObject->nodeId();
            for (auto portType : {QtNodes::PortType::In, QtNodes::PortType::Out}) {
                const auto countRole = portType == QtNodes::PortType::In ? QtNodes::NodeRole::InPortCount
                                                                         : QtNodes::NodeRole::OutPortCount;
                const unsigned int count = graphModel.nodeData(nodeId, countRole).toUInt();
                for (unsigned int i = 0; i < count; ++i) {
                    const QPointF portPos =
                        geometry.portScenePosition(nodeId, portType, i, nodeObject->sceneTransform());
                    if (QLineF(portPos, scenePos).length() > hitRadius) {
                        continue;
                    }
                    const QString caption =
                        graphModel.portData(nodeId, portType, i, QtNodes::PortRole::Caption).toString();
                    const auto dataType = graphModel.portData(nodeId, portType, i, QtNodes::PortRole::DataType)
                                              .value<QtNodes::NodeDataType>();
                    tooltip = QString("%1端口「%2」 · 数据类型：%3")
                                  .arg(portType == QtNodes::PortType::In ? "输入" : "输出",
                                       caption.isEmpty() ? dataType.name : caption,
                                       dataType.name);
                    break;
                }
                if (!tooltip.isEmpty()) {
                    break;
                }
            }
            if (!tooltip.isEmpty()) {
                break;
            }
        }

        if (tooltip.isEmpty()) {
            if (!portTooltipVisible_) {
                return;
            }
            portTooltipVisible_ = false;
            QToolTip::hideText();
            return;
        }
        portTooltipVisible_ = true;
        QToolTip::showText(viewport()->mapToGlobal(event->position().toPoint()), tooltip, viewport());
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier && quickPalette_) {
            quickPalette_(mapToScene(viewport()->rect().center()));
            event->accept();
            return;
        }
        QtNodes::GraphicsView::keyPressEvent(event);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        double steps = 0.0;
        if (!event->pixelDelta().isNull()) {
            steps = event->pixelDelta().y() / 240.0;
        } else if (event->angleDelta().y() != 0) {
            steps = event->angleDelta().y() / 120.0;
        }

        if (std::abs(steps) < 0.001) {
            QtNodes::GraphicsView::wheelEvent(event);
            return;
        }

        if (wheelZoomRequested_) {
            wheelZoomRequested_(steps);
            event->accept();
            return;
        }

        QtNodes::GraphicsView::wheelEvent(event);
    }

    void dragEnterEvent(QDragEnterEvent* event) override
    {
        // 仅接受拖放，不再绘制落点预测框（按用户要求彻底删除拖拽预测位置）。
        if (event->mimeData()->hasFormat("application/x-imagenode-type")) {
            event->acceptProposedAction();
            return;
        }
        // 外部文件：图片自动创建「读入图片」节点，.json 直接打开 workflow。
        if (!droppableLocalFiles(event->mimeData()).isEmpty()) {
            event->acceptProposedAction();
            return;
        }
        QtNodes::GraphicsView::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent* event) override
    {
        if (event->mimeData()->hasFormat("application/x-imagenode-type")
            || !droppableLocalFiles(event->mimeData()).isEmpty()) {
            event->acceptProposedAction();
            return;
        }
        QtNodes::GraphicsView::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent* event) override
    {
        if (event->mimeData()->hasFormat("application/x-imagenode-type") && nodeDropped_) {
            const QString typeName = QString::fromUtf8(event->mimeData()->data("application/x-imagenode-type"));
            if (typeName.trimmed().isEmpty() || !isKnownNodeType(typeName)) {
                event->ignore();
                return;
            }
            const QPointF scenePos = mapToScene(event->position().toPoint());
            auto callback = nodeDropped_;
            event->acceptProposedAction();
            QTimer::singleShot(0, this, [callback = std::move(callback), typeName, scenePos] {
                if (callback) {
                    callback(typeName, scenePos);
                }
            });
            return;
        }
        const QStringList files = droppableLocalFiles(event->mimeData());
        if (!files.isEmpty() && filesDropped_) {
            const QPointF scenePos = mapToScene(event->position().toPoint());
            auto callback = filesDropped_;
            event->acceptProposedAction();
            // 与节点拖放一致：延后到事件返回后处理，避免在 drop 回调里重建 scene。
            QTimer::singleShot(0, this, [callback = std::move(callback), files, scenePos] {
                if (callback) {
                    callback(files, scenePos);
                }
            });
            return;
        }
        QtNodes::GraphicsView::dropEvent(event);
    }

    void contextMenuEvent(QContextMenuEvent* event) override
    {
        // 让事件传到节点项，触发 nodeContextMenu 信号（弹 App 的中文节点菜单）；
        // 故意只调用祖父类 QGraphicsView，不调用 QtNodes::GraphicsView::contextMenuEvent
        // —— 后者在右键节点/组时还会弹出自带的英文 Copy/Cut 菜单，与中文菜单重叠。
        QGraphicsView::contextMenuEvent(event);
        // 仅当右键落在空白画布（没有任何 item）时，弹出 App 的中文场景菜单。
        if (items(event->pos()).isEmpty()) {
            if (auto* menu = nodeScene()->createSceneMenu(mapToScene(event->pos()))) {
                menu->exec(event->globalPos());
            }
        }
    }

private:
    double* uiScale_ = nullptr;
    std::function<void()> deleteSelection_;
    std::function<void()> copySelection_;
    std::function<void(const QPointF&)> quickPalette_;
    std::function<void(const QString&, const QPointF&)> nodeDropped_;
    std::function<void(double)> wheelZoomRequested_;
    std::function<void(const QStringList&, const QPointF&)> filesDropped_;
    QVector<QLineF> alignmentGuides_;
    bool portTooltipVisible_ = false;
};

class WorkflowGraphicsScene final : public QtNodes::DataFlowGraphicsScene {
public:
    WorkflowGraphicsScene(QtNodes::DataFlowGraphModel& graphModel,
                          std::function<void(const QPointF&)> quickPalette,
                          std::function<QMenu*(const QPointF&)> sceneContextMenu,
                          QObject* parent)
        : QtNodes::DataFlowGraphicsScene(graphModel, parent),
          quickPalette_(std::move(quickPalette)),
          sceneContextMenu_(std::move(sceneContextMenu))
    {
        setGroupingEnabled(false);
    }

    QMenu* createSceneMenu(const QPointF scenePos) override
    {
        if (sceneContextMenu_) {
            return sceneContextMenu_(scenePos);
        }
        if (quickPalette_) {
            quickPalette_(scenePos);
        }
        return nullptr;
    }

private:
    std::function<void(const QPointF&)> quickPalette_;
    std::function<QMenu*(const QPointF&)> sceneContextMenu_;
};

}

WorkflowCanvas::WorkflowCanvas(QWidget* owner, double uiScale, Callbacks callbacks)
    : owner_(owner), uiScale_(uiScale), callbacks_(std::move(callbacks))
{
    applyQtNodesStyle();
    view_ = new WorkflowGraphicsView(nullptr,
                                     &uiScale_,
                                     callbacks_.deleteSelection,
                                     callbacks_.copySelection,
                                     callbacks_.quickPalette,
                                     callbacks_.nodeDropped,
                                     callbacks_.wheelZoomRequested,
                                     callbacks_.filesDropped,
                                     owner_);
    view_->setObjectName("workflowView");
    view_->viewport()->setObjectName("workflowViewport");
    view_->setFrameShape(QFrame::NoFrame);
    view_->setScaleRange(0.25, 3.0);
}

WorkflowCanvas::~WorkflowCanvas()
{
    // 析构时成员 scene_ 会逐个删除 NodeGraphicsObject 并触发 selectionChanged 等信号；
    // 此时仍连着的 updateExpandedNodes 回调会访问正在析构的 scene（unique_ptr 析构不会
    // 像 reset() 那样先置空指针，故 if(!scene_) 守卫失效）而崩溃。先断开 scene 的全部连接。
    if (scene_) {
        scene_->disconnect();
    }
}

QGraphicsView* WorkflowCanvas::view() const
{
    return view_;
}

QGraphicsScene* WorkflowCanvas::scene() const
{
    return scene_.get();
}

void WorkflowCanvas::rebuild(WorkflowGraph& graph,
                             const QMap<QString, NodeExecutionState>& runStates,
                             const QMap<QString, qint64>& elapsedMs)
{
    graph_ = &graph;
    syncing_ = true;
    // 主题可能已切换，重建场景时按当前 palette 重新应用 Qt Nodes 全局样式。
    applyQtNodesStyle();
    workflowToQtNode_.clear();
    qtToWorkflowNode_.clear();
    registry_ = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    for (const auto& descriptor : NodeFactory::instance().descriptors()) {
        registry_->registerModel([this, typeName = descriptor.typeName] {
            return createDelegate(typeName);
        }, descriptor.category);
    }

    static_cast<WorkflowGraphicsView*>(view_)->clearTransientItems();
    static_cast<QGraphicsView*>(view_)->setScene(nullptr);
    scene_.reset();
    graphModel_ = std::make_unique<QtNodes::DataFlowGraphModel>(registry_);

    for (const auto& record : graph.nodes()) {
        pendingWorkflowNodeId_ = record.id;
        pendingNodeType_ = record.node->typeName();
        const QtNodes::NodeId nodeId = graphModel_->addNode(record.node->typeName());
        pendingWorkflowNodeId_.clear();
        pendingNodeType_.clear();
        if (nodeId == QtNodes::InvalidNodeId) {
            continue;
        }
        workflowToQtNode_.insert(record.id, nodeId);
        qtToWorkflowNode_.insert(nodeId, record.id);
        graphModel_->setNodeData(nodeId, QtNodes::NodeRole::Position, record.position);
    }

    for (const auto& edge : graph.edges()) {
        const QtNodes::ConnectionId connection = connectionForEdge(edge);
        if (connection.outNodeId != QtNodes::InvalidNodeId && connection.inNodeId != QtNodes::InvalidNodeId &&
            graphModel_->connectionPossible(connection)) {
            graphModel_->addConnection(connection);
        }
    }

    scene_ = std::make_unique<WorkflowGraphicsScene>(*graphModel_, callbacks_.quickPalette, callbacks_.sceneContextMenu, owner_);
    scene_->setSceneRect(-2000, -2000, 4000, 4000);
    scene_->setNodePainter(std::make_unique<WorkflowNodePainter>());
    scene_->setConnectionPainter(std::make_unique<WorkflowConnectionPainter>());
    view_->setScene(scene_.get());
    connectSceneCallbacks();

    for (auto it = workflowToQtNode_.cbegin(); it != workflowToQtNode_.cend(); ++it) {
        setExecutionState(it.key(), runStates.value(it.key(), NodeExecutionState::NotExecuted));
        setElapsedMs(it.key(), elapsedMs.value(it.key(), -1));
    }
    syncing_ = false;
    updateExpandedNodes();
}

QMap<QString, QGraphicsItem*> WorkflowCanvas::nodeItems() const
{
    QMap<QString, QGraphicsItem*> items;
    if (!scene_) {
        return items;
    }
    for (auto it = workflowToQtNode_.cbegin(); it != workflowToQtNode_.cend(); ++it) {
        items.insert(it.key(), scene_->nodeGraphicsObject(it.value()));
    }
    return items;
}

QStringList WorkflowCanvas::selectedNodeIds() const
{
    QStringList ids;
    if (!scene_) {
        return ids;
    }
    for (const QtNodes::NodeId nodeId : scene_->selectedNodes()) {
        const QString workflowId = qtToWorkflowNode_.value(nodeId);
        if (!workflowId.isEmpty()) {
            ids.append(workflowId);
        }
    }
    return ids;
}

QVector<int> WorkflowCanvas::selectedEdgeIndexes(const WorkflowGraph& graph) const
{
    QVector<int> indexes;
    if (!scene_) {
        return indexes;
    }
    for (auto* item : scene_->selectedItems()) {
        auto* connection = qgraphicsitem_cast<QtNodes::ConnectionGraphicsObject*>(item);
        if (!connection) {
            continue;
        }
        const Edge edge = edgeForConnection(connection->connectionId());
        for (int index = 0; index < graph.edges().size(); ++index) {
            const Edge current = graph.edges().at(index);
            if (current.fromNode == edge.fromNode && current.fromPort == edge.fromPort &&
                current.toNode == edge.toNode && current.toPort == edge.toPort) {
                indexes.append(index);
                break;
            }
        }
    }
    return indexes;
}

bool WorkflowCanvas::selectNode(const QString& workflowNodeId)
{
    if (!scene_ || !workflowToQtNode_.contains(workflowNodeId)) {
        return false;
    }
    scene_->clearSelection();
    auto* item = scene_->nodeGraphicsObject(workflowToQtNode_.value(workflowNodeId));
    if (!item) {
        return false;
    }
    item->setSelected(true);
    updateExpandedNodes();
    return true;
}

bool WorkflowCanvas::focusNode(const QString& workflowNodeId)
{
    if (!selectNode(workflowNodeId)) {
        return false;
    }
    view_->centerOn(scene_->nodeGraphicsObject(workflowToQtNode_.value(workflowNodeId)));
    return true;
}

void WorkflowCanvas::setExecutionState(const QString& workflowNodeId, NodeExecutionState state)
{
    if (!graphModel_ || !workflowToQtNode_.contains(workflowNodeId)) {
        return;
    }
    if (auto* delegate = graphModel_->delegateModel<WorkflowNodeDelegate>(workflowToQtNode_.value(workflowNodeId))) {
        delegate->setExecutionState(state);
    }
}

void WorkflowCanvas::setElapsedMs(const QString& workflowNodeId, qint64 elapsedMs)
{
    if (!graphModel_ || !workflowToQtNode_.contains(workflowNodeId)) {
        return;
    }
    if (auto* delegate = graphModel_->delegateModel<WorkflowNodeDelegate>(workflowToQtNode_.value(workflowNodeId))) {
        delegate->setElapsedMs(elapsedMs);
    }
}

void WorkflowCanvas::setAnimationPhase(const QString& workflowNodeId, int phase)
{
    if (!graphModel_ || !workflowToQtNode_.contains(workflowNodeId)) {
        return;
    }
    if (auto* delegate = graphModel_->delegateModel<WorkflowNodeDelegate>(workflowToQtNode_.value(workflowNodeId))) {
        delegate->setAnimationPhase(phase);
    }
}

std::unique_ptr<WorkflowNodeDelegate> WorkflowCanvas::createDelegate(const QString& typeName)
{
    QString workflowId;
    QSharedPointer<ImageNode> node;
    if (graph_ && pendingNodeType_ == typeName && !pendingWorkflowNodeId_.isEmpty()) {
        workflowId = pendingWorkflowNodeId_;
        node = graph_->node(workflowId);
    }
    if (!node) {
        const auto created = NodeFactory::instance().create(typeName);
        if (created.isOk()) {
            node = created.value();
        }
    }
    return std::make_unique<WorkflowNodeDelegate>(workflowId,
                                                   categoryForType(typeName),
                                                   node,
                                                   uiScale_,
                                                   callbacks_.parameterChanged);
}

Edge WorkflowCanvas::edgeForConnection(const QtNodes::ConnectionId& connection) const
{
    const QString fromId = qtToWorkflowNode_.value(connection.outNodeId);
    const QString toId = qtToWorkflowNode_.value(connection.inNodeId);
    return Edge{fromId,
                graph_ ? portName(graph_->node(fromId), int(connection.outPortIndex), PortDirection::Output) : QString(),
                toId,
                graph_ ? portName(graph_->node(toId), int(connection.inPortIndex), PortDirection::Input) : QString()};
}

QtNodes::ConnectionId WorkflowCanvas::connectionForEdge(const Edge& edge) const
{
    const QtNodes::NodeId outputId = workflowToQtNode_.value(edge.fromNode, QtNodes::InvalidNodeId);
    const QtNodes::NodeId inputId = workflowToQtNode_.value(edge.toNode, QtNodes::InvalidNodeId);
    const int outputPort = graph_ ? portIndex(graph_->node(edge.fromNode), edge.fromPort, PortDirection::Output) : -1;
    const int inputPort = graph_ ? portIndex(graph_->node(edge.toNode), edge.toPort, PortDirection::Input) : -1;
    if (outputPort < 0 || inputPort < 0) {
        return {QtNodes::InvalidNodeId, QtNodes::InvalidPortIndex, QtNodes::InvalidNodeId, QtNodes::InvalidPortIndex};
    }
    return {outputId, QtNodes::PortIndex(outputPort), inputId, QtNodes::PortIndex(inputPort)};
}

void WorkflowCanvas::updateExpandedNodes()
{
    if (!graphModel_) {
        return;
    }
    const QStringList selected = selectedNodeIds();
    for (auto it = workflowToQtNode_.cbegin(); it != workflowToQtNode_.cend(); ++it) {
        if (auto* delegate = graphModel_->delegateModel<WorkflowNodeDelegate>(it.value())) {
            delegate->setExpanded(selected.contains(it.key()));
        }
    }
}

void WorkflowCanvas::connectSceneCallbacks()
{
    if (!scene_ || !graphModel_) {
        return;
    }
    QObject::connect(scene_.get(), &QGraphicsScene::selectionChanged, owner_, [this] {
        updateExpandedNodes();
        if (callbacks_.selectionChanged) {
            callbacks_.selectionChanged(selectedNodeIds().value(0));
        }
    });
    QObject::connect(scene_.get(), &QtNodes::BasicGraphicsScene::nodeContextMenu, owner_,
                     [this](QtNodes::NodeId nodeId, const QPointF& scenePos) {
        if (!callbacks_.nodeContextMenu || !view_) {
            return;
        }
        callbacks_.nodeContextMenu(qtToWorkflowNode_.value(nodeId),
                                   view_->viewport()->mapToGlobal(view_->mapFromScene(scenePos)));
    });
    QObject::connect(scene_.get(), &QtNodes::BasicGraphicsScene::nodeDoubleClicked, owner_,
                     [this](QtNodes::NodeId nodeId) {
        if (callbacks_.nodeDoubleClicked) {
            callbacks_.nodeDoubleClicked(qtToWorkflowNode_.value(nodeId));
        }
    });
    // 拖动过程中实时计算与其他节点的对齐辅助线（左/中/右、上/中/下，阈值内画虚线）。
    QObject::connect(scene_.get(), &QtNodes::BasicGraphicsScene::nodeMoved, owner_,
                     [this](QtNodes::NodeId nodeId, const QPointF& newLocation) {
        auto* view = static_cast<WorkflowGraphicsView*>(view_);
        if (syncing_ || !scene_ || !view) {
            return;
        }
        constexpr double kSnapThreshold = 5.0;
        const QSizeF movingSize = scene_->nodeGeometry().size(nodeId);
        const QRectF moving(newLocation, movingSize);
        const double movingXs[] = {moving.left(), moving.center().x(), moving.right()};
        const double movingYs[] = {moving.top(), moving.center().y(), moving.bottom()};

        QVector<QLineF> guides;
        for (auto it = workflowToQtNode_.cbegin(); it != workflowToQtNode_.cend(); ++it) {
            if (it.value() == nodeId) {
                continue;
            }
            auto* other = scene_->nodeGraphicsObject(it.value());
            if (!other) {
                continue;
            }
            const QRectF rect(other->pos(), scene_->nodeGeometry().size(it.value()));
            const double otherXs[] = {rect.left(), rect.center().x(), rect.right()};
            const double otherYs[] = {rect.top(), rect.center().y(), rect.bottom()};
            for (double mx : movingXs) {
                for (double ox : otherXs) {
                    if (std::abs(mx - ox) <= kSnapThreshold) {
                        const double top = std::min(moving.top(), rect.top()) - 24.0;
                        const double bottom = std::max(moving.bottom(), rect.bottom()) + 24.0;
                        guides.append(QLineF(ox, top, ox, bottom));
                    }
                }
            }
            for (double my : movingYs) {
                for (double oy : otherYs) {
                    if (std::abs(my - oy) <= kSnapThreshold) {
                        const double left = std::min(moving.left(), rect.left()) - 24.0;
                        const double right = std::max(moving.right(), rect.right()) + 24.0;
                        guides.append(QLineF(left, oy, right, oy));
                    }
                }
            }
            if (guides.size() >= 6) {
                break;  // 限制数量，避免密集图上画满参考线
            }
        }
        view->setAlignmentGuides(std::move(guides));
    });
    QObject::connect(scene_.get(), &QtNodes::BasicGraphicsScene::nodeClicked, owner_,
                     [this](QtNodes::NodeId nodeId) {
        static_cast<WorkflowGraphicsView*>(view_)->setAlignmentGuides({});
        if (syncing_ || !graph_ || !callbacks_.nodeMoved) {
            return;
        }
        const QString workflowId = qtToWorkflowNode_.value(nodeId);
        const auto* record = graph_->nodeRecord(workflowId);
        auto* item = scene_->nodeGraphicsObject(nodeId);
        if (!record || !item || samePoint(record->position, item->pos())) {
            return;
        }
        const QPointF before = record->position;
        graph_->setNodePosition(workflowId, item->pos());
        graphModel_->setNodeData(nodeId, QtNodes::NodeRole::Position, item->pos());
        callbacks_.nodeMoved(workflowId, before, item->pos());
    });
    QObject::connect(graphModel_.get(), &QtNodes::AbstractGraphModel::connectionCreated, owner_,
                     [this](const QtNodes::ConnectionId connection) {
        if (syncing_ || !callbacks_.edgeAdded) {
            return;
        }
        const Edge edge = edgeForConnection(connection);
        if (edge.fromNode.isEmpty() || edge.toNode.isEmpty() || !callbacks_.edgeAdded(edge)) {
            graphModel_->deleteConnection(connection);
        }
    });
    QObject::connect(graphModel_.get(), &QtNodes::AbstractGraphModel::connectionDeleted, owner_,
                     [this](const QtNodes::ConnectionId connection) {
        if (!syncing_ && callbacks_.edgeRemoved) {
            callbacks_.edgeRemoved(edgeForConnection(connection));
        }
    });
}
