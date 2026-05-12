#pragma once

#include "core/NodeData.h"
#include "core/Result.h"

#include <QMap>
#include <QStringList>
#include <QVector>

#include <functional>

class WorkflowGraph;

enum class NodeExecutionState {
    NotExecuted,
    Running,
    Succeeded,
    Failed,
    CacheHit
};

struct NodeExecutionSummary {
    QString nodeId;
    QString displayName;
    NodeExecutionState state = NodeExecutionState::NotExecuted;
    QString message;
    qint64 elapsedMs = -1;
};

struct ExecutionResult {
    QMap<QString, QMap<QString, NodeData>> nodeOutputs;
    QStringList log;
    QVector<NodeExecutionSummary> nodeSummaries;
    QString failedNodeId;
};

class ExecutionEngine {
public:
    using NodeEventCallback = std::function<void(const NodeExecutionSummary&)>;
    using ExternalInputMap = QMap<QString, NodeData>;

    Result<ExecutionResult> execute(const WorkflowGraph& graph);
    Result<ExecutionResult> execute(const WorkflowGraph& graph, const NodeEventCallback& onNodeEvent);
    Result<ExecutionResult> executeForNode(const WorkflowGraph& graph,
                                           const QString& targetNodeId,
                                           const NodeEventCallback& onNodeEvent = {});
    Result<ExecutionResult> executeWithExternalInputs(const WorkflowGraph& graph,
                                                      const ExternalInputMap& externalInputs,
                                                      const NodeEventCallback& onNodeEvent = {});
    void clearCache();
    const ExecutionResult& lastResult() const { return lastResult_; }

private:
    struct CacheEntry {
        QString signature;
        QMap<QString, NodeData> outputs;
    };

    Result<ExecutionResult> executeOrderedNodes(const WorkflowGraph& graph,
                                                const QStringList& nodeOrder,
                                                const ExternalInputMap& externalInputs,
                                                const NodeEventCallback& onNodeEvent);

    QMap<QString, CacheEntry> cache_;
    ExecutionResult lastResult_;
};
