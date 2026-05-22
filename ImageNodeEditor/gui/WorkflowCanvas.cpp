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
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QVariant>

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

void paintCanvasGrid(QPainter* painter, const QRectF& rect, double uiScale)
{
    const auto colors = AppTheme::colors();
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setColorAt(0, colors.canvasTop);
    gradient.setColorAt(1, colors.canvasBottom);
    painter->fillRect(rect, gradient);

    const qreal step = 26.0 * uiScale;
    const qreal dotRadius = std::max<qreal>(1.0, 1.15 * uiScale);
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
                         QWidget* parent)
        : QtNodes::GraphicsView(scene, parent),
          uiScale_(uiScale),
          deleteSelection_(std::move(deleteSelection)),
          copySelection_(std::move(copySelection)),
          quickPalette_(std::move(quickPalette))
    {
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

private:
    double* uiScale_ = nullptr;
    std::function<void()> deleteSelection_;
    std::function<void()> copySelection_;
    std::function<void(const QPointF&)> quickPalette_;
};

class WorkflowGraphicsScene final : public QtNodes::DataFlowGraphicsScene {
public:
    WorkflowGraphicsScene(QtNodes::DataFlowGraphModel& graphModel,
                          std::function<void(const QPointF&)> quickPalette,
                          QObject* parent)
        : QtNodes::DataFlowGraphicsScene(graphModel, parent), quickPalette_(std::move(quickPalette))
    {
        setGroupingEnabled(false);
    }

    QMenu* createSceneMenu(const QPointF scenePos) override
    {
        if (quickPalette_) {
            quickPalette_(scenePos);
        }
        return nullptr;
    }

private:
    std::function<void(const QPointF&)> quickPalette_;
};

}

WorkflowCanvas::WorkflowCanvas(QWidget* owner, double uiScale, Callbacks callbacks)
    : owner_(owner), uiScale_(uiScale), callbacks_(std::move(callbacks))
{
    view_ = new WorkflowGraphicsView(nullptr,
                                     &uiScale_,
                                     callbacks_.deleteSelection,
                                     callbacks_.copySelection,
                                     callbacks_.quickPalette,
                                     owner_);
    view_->setObjectName("workflowView");
    view_->viewport()->setObjectName("workflowViewport");
    view_->setFrameShape(QFrame::NoFrame);
    view_->setScaleRange(0.25, 3.0);
}

WorkflowCanvas::~WorkflowCanvas() = default;

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
    workflowToQtNode_.clear();
    qtToWorkflowNode_.clear();
    registry_ = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    for (const auto& descriptor : NodeFactory::instance().descriptors()) {
        registry_->registerModel([this, typeName = descriptor.typeName] {
            return createDelegate(typeName);
        }, descriptor.category);
    }

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

    scene_ = std::make_unique<WorkflowGraphicsScene>(*graphModel_, callbacks_.quickPalette, owner_);
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
