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
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
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
                         QWidget* parent)
        : QtNodes::GraphicsView(scene, parent),
          uiScale_(uiScale),
          deleteSelection_(std::move(deleteSelection)),
          copySelection_(std::move(copySelection)),
          quickPalette_(std::move(quickPalette)),
          nodeDropped_(std::move(nodeDropped)),
          wheelZoomRequested_(std::move(wheelZoomRequested))
    {
        setAcceptDrops(true);
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
        // 已无落点预测框等临时图元，保留为空操作以兼容 rebuild() 的调用点。
    }

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override
    {
        paintCanvasGrid(painter, rect, uiScale_ ? *uiScale_ : 1.0);
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
        QtNodes::GraphicsView::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent* event) override
    {
        if (event->mimeData()->hasFormat("application/x-imagenode-type")) {
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
    QObject::connect(scene_.get(), &QtNodes::BasicGraphicsScene::nodeClicked, owner_,
                     [this](QtNodes::NodeId nodeId) {
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
