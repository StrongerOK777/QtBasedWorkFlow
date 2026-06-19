#pragma once

#include "core/NodeData.h"
#include "core/Result.h"

#include <QMap>
#include <QStringList>
#include <QVector>

#include <atomic>
#include <functional>
#include <memory>

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

    // 取消令牌：引擎按值拷贝进工作线程，因此用 shared_ptr 共享同一标志。
    // 取消粒度为「调度轮」：已开始执行的节点会自然结束，之后不再调度新节点。
    void setCancelFlag(std::shared_ptr<std::atomic<bool>> flag) { cancelFlag_ = std::move(flag); }
    bool isCancelled() const { return cancelFlag_ && cancelFlag_->load(std::memory_order_relaxed); }

    static constexpr quint64 kMaxCacheBytes = 512ull * 1024 * 1024; // 缓存总量上限，超出按 LRU 逐出

private:
    struct CacheEntry {
        QString signature;
        QMap<QString, NodeData> outputs;
        quint64 bytes = 0;
        quint64 lastUse = 0;
    };

    Result<ExecutionResult> executeOrderedNodes(const WorkflowGraph& graph,
                                                const QStringList& nodeOrder,
                                                const ExternalInputMap& externalInputs,
                                                const NodeEventCallback& onNodeEvent);
    void insertCacheEntry(const QString& nodeId, const QString& signature, const QMap<QString, NodeData>& outputs);

    QMap<QString, CacheEntry> cache_;
    quint64 cacheBytes_ = 0;
    quint64 cacheTick_ = 0;
    std::shared_ptr<std::atomic<bool>> cancelFlag_;
    ExecutionResult lastResult_;
};
