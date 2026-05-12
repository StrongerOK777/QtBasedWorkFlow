#include "workflow/ExecutionEngine.h"

#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"
#include "workflow/WorkflowValidator.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include <algorithm>
#include <future>
#include <vector>

namespace {

QString compactJson(const QJsonObject& object)
{
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QString inputFileIdentity(const QJsonObject& params)
{
    const QString path = params.value("filePath").toString();
    if (path.isEmpty()) {
        return "file=";
    }

    const QFileInfo info(path);
    QStringList parts;
    parts << QString("file=%1").arg(info.absoluteFilePath());
    parts << QString("exists=%1").arg(info.exists() ? "1" : "0");
    if (info.exists()) {
        parts << QString("size=%1").arg(info.size());
        parts << QString("mtime=%1").arg(info.lastModified().toMSecsSinceEpoch());
    }
    return parts.join("|");
}

QString nodeSignature(const QString& nodeId, const QSharedPointer<ImageNode>& node, const QStringList& inputParts)
{
    QStringList parts;
    const QJsonObject params = node->saveParams();
    parts << QString("id=%1").arg(nodeId);
    parts << QString("type=%1").arg(node->typeName());
    parts << QString("params=%1").arg(compactJson(params));
    if (node->typeName() == "ImageInput") {
        parts << inputFileIdentity(params);
    }
    parts << inputParts;
    const QByteArray bytes = parts.join("\n").toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

}

Result<ExecutionResult> ExecutionEngine::execute(const WorkflowGraph& graph)
{
    return execute(graph, {});
}

Result<ExecutionResult> ExecutionEngine::execute(const WorkflowGraph& graph, const NodeEventCallback& onNodeEvent)
{
    WorkflowValidator validator;
    lastResult_ = {};
    auto valid = validator.validate(graph);
    if (valid.isFail()) {
        return Result<ExecutionResult>::fail(valid.error());
    }
    auto order = validator.topologicalOrder(graph);
    if (order.isFail()) {
        return Result<ExecutionResult>::fail(order.error());
    }
    return executeOrderedNodes(graph, order.value(), onNodeEvent);
}

Result<ExecutionResult> ExecutionEngine::executeForNode(const WorkflowGraph& graph,
                                                        const QString& targetNodeId,
                                                        const NodeEventCallback& onNodeEvent)
{
    WorkflowValidator validator;
    lastResult_ = {};
    if (!graph.containsNode(targetNodeId)) {
        return Result<ExecutionResult>::fail(QString("预览目标节点不存在：%1").arg(targetNodeId));
    }
    auto order = validator.topologicalOrder(graph);
    if (order.isFail()) {
        return Result<ExecutionResult>::fail(order.error());
    }

    QSet<QString> requiredNodes;
    requiredNodes.insert(targetNodeId);
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& edge : graph.edges()) {
            if (requiredNodes.contains(edge.toNode) && !requiredNodes.contains(edge.fromNode)) {
                requiredNodes.insert(edge.fromNode);
                changed = true;
            }
        }
    }

    QStringList previewOrder;
    for (const QString& nodeId : order.value()) {
        if (requiredNodes.contains(nodeId)) {
            previewOrder.append(nodeId);
        }
    }
    return executeOrderedNodes(graph, previewOrder, onNodeEvent);
}

Result<ExecutionResult> ExecutionEngine::executeOrderedNodes(const WorkflowGraph& graph,
                                                             const QStringList& nodeOrder,
                                                             const NodeEventCallback& onNodeEvent)
{
    ExecutionResult result;
    QMap<QString, QString> nodeSignatures;
    auto record = [&](const NodeExecutionSummary& summary) {
        result.nodeSummaries.append(summary);
        if (onNodeEvent) {
            onNodeEvent(summary);
        }
    };

    struct Job {
        QString nodeId;
        QString displayName;
        QString signature;
        bool cacheable = false;
        std::future<Result<QMap<QString, NodeData>>> future;
    };

    QSet<QString> pending;
    QMap<QString, int> indegree;
    QMap<QString, QStringList> downstream;
    for (const QString& nodeId : nodeOrder) {
        pending.insert(nodeId);
        indegree.insert(nodeId, 0);
    }
    for (const auto& edge : graph.edges()) {
        if (!pending.contains(edge.fromNode) || !pending.contains(edge.toNode)) {
            continue;
        }
        ++indegree[edge.toNode];
        downstream[edge.fromNode].append(edge.toNode);
    }

    QStringList ready;
    for (const QString& nodeId : nodeOrder) {
        if (indegree.value(nodeId) == 0) {
            ready.append(nodeId);
        }
    }

    while (!ready.isEmpty()) {
        std::vector<Job> jobs;
        QStringList completedThisRound;

        for (const QString& nodeId : ready) {
            auto node = graph.node(nodeId);
            if (!node) {
                lastResult_ = result;
                return Result<ExecutionResult>::fail(QString("执行节点不存在：%1").arg(nodeId));
            }

            QMap<QString, NodeData> inputs;
            QStringList inputParts;
            for (const auto& edge : graph.edges()) {
                if (edge.toNode == nodeId) {
                    if (!result.nodeOutputs.contains(edge.fromNode) || !result.nodeOutputs[edge.fromNode].contains(edge.fromPort)) {
                        lastResult_ = result;
                        return Result<ExecutionResult>::fail(QString("上游输出不存在：%1.%2").arg(edge.fromNode, edge.fromPort));
                    }
                    inputs.insert(edge.toPort, result.nodeOutputs[edge.fromNode][edge.fromPort]);
                    inputParts.append(QString("%1=%2.%3:%4")
                                          .arg(edge.toPort, edge.fromNode, edge.fromPort, nodeSignatures.value(edge.fromNode)));
                }
            }
            std::sort(inputParts.begin(), inputParts.end());
            const QString signature = nodeSignature(nodeId, node, inputParts);
            nodeSignatures.insert(nodeId, signature);

            record(NodeExecutionSummary{nodeId, node->displayName(), NodeExecutionState::Running,
                                        QString("执行节点 %1 (%2)").arg(nodeId, node->displayName())});

            const auto cached = cache_.constFind(nodeId);
            if (node->isCacheable() && cached != cache_.constEnd() && cached.value().signature == signature) {
                result.nodeOutputs.insert(nodeId, cached.value().outputs);
                result.log.append(QString("复用缓存 %1 (%2)").arg(nodeId, node->displayName()));
                record(NodeExecutionSummary{nodeId, node->displayName(), NodeExecutionState::CacheHit,
                                            QString("复用缓存 %1").arg(nodeId)});
                completedThisRound.append(nodeId);
                continue;
            }

            result.log.append(QString("执行节点 %1 (%2)").arg(nodeId, node->displayName()));
            jobs.push_back(Job{nodeId,
                               node->displayName(),
                               signature,
                               node->isCacheable(),
                               std::async(std::launch::async, [node, inputs]() mutable {
                                   return node->execute(inputs);
                               })});
        }

        for (auto& job : jobs) {
            auto outputs = job.future.get();
            if (outputs.isFail()) {
                result.log.append(QString("节点失败 %1：%2").arg(job.nodeId, outputs.error()));
                result.failedNodeId = job.nodeId;
                record(NodeExecutionSummary{job.nodeId, job.displayName, NodeExecutionState::Failed,
                                            QString("节点失败 %1：%2").arg(job.nodeId, outputs.error())});
                lastResult_ = result;
                return Result<ExecutionResult>::fail(QString("节点 %1 (%2)：%3").arg(job.nodeId, job.displayName, outputs.error()));
            }
            result.nodeOutputs.insert(job.nodeId, outputs.value());
            if (job.cacheable) {
                cache_.insert(job.nodeId, CacheEntry{job.signature, outputs.value()});
            }
            result.log.append(QString("节点完成 %1").arg(job.nodeId));
            record(NodeExecutionSummary{job.nodeId, job.displayName, NodeExecutionState::Succeeded,
                                        QString("节点完成 %1").arg(job.nodeId)});
            completedThisRound.append(job.nodeId);
        }

        QStringList nextReady;
        for (const QString& nodeId : completedThisRound) {
            pending.remove(nodeId);
            for (const QString& next : downstream.value(nodeId)) {
                --indegree[next];
                if (indegree.value(next) == 0) {
                    nextReady.append(next);
                }
            }
        }
        ready.clear();
        for (const QString& nodeId : nodeOrder) {
            if (nextReady.contains(nodeId)) {
                ready.append(nodeId);
            }
        }
    }

    if (!pending.isEmpty()) {
        lastResult_ = result;
        return Result<ExecutionResult>::fail("工作流包含无法调度的节点");
    }
    lastResult_ = result;
    return Result<ExecutionResult>::ok(result);
}

void ExecutionEngine::clearCache()
{
    cache_.clear();
    lastResult_ = {};
}
