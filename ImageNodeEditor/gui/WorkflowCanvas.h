#pragma once

#include "core/Edge.h"
#include "workflow/ExecutionEngine.h"

#include <QMap>
#include <QStringList>
#include <QVector>
#include <functional>
#include <memory>

class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QMenu;
class QWidget;
class WorkflowGraph;

namespace QtNodes {
class DataFlowGraphModel;
class DataFlowGraphicsScene;
class GraphicsView;
class NodeDelegateModelRegistry;
using NodeId = unsigned int;
struct ConnectionId;
}

class WorkflowCanvas final {
public:
    struct Callbacks {
        std::function<bool(const Edge&)> edgeAdded;
        std::function<void(const Edge&)> edgeRemoved;
        std::function<void(const QString&, const QPointF&, const QPointF&)> nodeMoved;
        std::function<void(const QString&)> selectionChanged;
        std::function<void(const QString&, const QPoint&)> nodeContextMenu;
        std::function<void(const QString&)> nodeDoubleClicked;
        std::function<void()> deleteSelection;
        std::function<void()> copySelection;
        std::function<void(const QPointF&)> quickPalette;
        std::function<QMenu*(const QPointF&)> sceneContextMenu;
        std::function<void(const QString&, const QPointF&)> nodeDropped;
        std::function<void(double)> wheelZoomRequested;
        std::function<void(const QString&, const QString&, const QVariant&)> parameterChanged;
        // 外部文件拖入画布：图片创建「读入图片」节点、.json 打开 workflow。
        std::function<void(const QStringList&, const QPointF&)> filesDropped;
    };

    WorkflowCanvas(QWidget* owner, double uiScale, Callbacks callbacks);
    ~WorkflowCanvas();

    QGraphicsView* view() const;
    QGraphicsScene* scene() const;
    void rebuild(WorkflowGraph& graph,
                 const QMap<QString, NodeExecutionState>& runStates,
                 const QMap<QString, qint64>& elapsedMs);
    void setUiScale(double uiScale) { uiScale_ = uiScale; }

    QMap<QString, QGraphicsItem*> nodeItems() const;
    QStringList selectedNodeIds() const;
    QVector<int> selectedEdgeIndexes(const WorkflowGraph& graph) const;
    bool selectNode(const QString& workflowNodeId);
    bool focusNode(const QString& workflowNodeId);
    void setExecutionState(const QString& workflowNodeId, NodeExecutionState state);
    void setElapsedMs(const QString& workflowNodeId, qint64 elapsedMs);
    void setAnimationPhase(const QString& workflowNodeId, int phase);

private:
    std::unique_ptr<class WorkflowNodeDelegate> createDelegate(const QString& typeName);
    Edge edgeForConnection(const QtNodes::ConnectionId& connection) const;
    QtNodes::ConnectionId connectionForEdge(const Edge& edge) const;
    void updateExpandedNodes();
    void connectSceneCallbacks();

    QWidget* owner_ = nullptr;
    WorkflowGraph* graph_ = nullptr;
    double uiScale_ = 1.0;
    Callbacks callbacks_;
    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry_;
    std::unique_ptr<QtNodes::DataFlowGraphModel> graphModel_;
    std::unique_ptr<QtNodes::DataFlowGraphicsScene> scene_;
    QtNodes::GraphicsView* view_ = nullptr;
    QMap<QString, QtNodes::NodeId> workflowToQtNode_;
    QMap<QtNodes::NodeId, QString> qtToWorkflowNode_;
    QString pendingWorkflowNodeId_;
    QString pendingNodeType_;
    bool syncing_ = false;
};
