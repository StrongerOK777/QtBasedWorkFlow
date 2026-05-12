#include "workflow/ExecutionEngine.h"

#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"
#include "workflow/WorkflowValidator.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

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

    ExecutionResult result;
    QMap<QString, QString> nodeSignatures;
    auto record = [&](const NodeExecutionSummary& summary) {
        result.nodeSummaries.append(summary);
        if (onNodeEvent) {
            onNodeEvent(summary);
        }
    };

    for (const QString& nodeId : order.value()) {
        auto node = graph.node(nodeId);
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
            continue;
        }

        result.log.append(QString("执行节点 %1 (%2)").arg(nodeId, node->displayName()));
        auto outputs = node->execute(inputs);
        if (outputs.isFail()) {
            result.log.append(QString("节点失败 %1：%2").arg(nodeId, outputs.error()));
            result.failedNodeId = nodeId;
            record(NodeExecutionSummary{nodeId, node->displayName(), NodeExecutionState::Failed,
                                        QString("节点失败 %1：%2").arg(nodeId, outputs.error())});
            lastResult_ = result;
            return Result<ExecutionResult>::fail(QString("节点 %1 (%2)：%3").arg(nodeId, node->displayName(), outputs.error()));
        }
        result.nodeOutputs.insert(nodeId, outputs.value());
        if (node->isCacheable()) {
            cache_.insert(nodeId, CacheEntry{signature, outputs.value()});
        }
        result.log.append(QString("节点完成 %1").arg(nodeId));
        record(NodeExecutionSummary{nodeId, node->displayName(), NodeExecutionState::Succeeded,
                                    QString("节点完成 %1").arg(nodeId)});
    }
    lastResult_ = result;
    return Result<ExecutionResult>::ok(result);
}

void ExecutionEngine::clearCache()
{
    cache_.clear();
    lastResult_ = {};
}
