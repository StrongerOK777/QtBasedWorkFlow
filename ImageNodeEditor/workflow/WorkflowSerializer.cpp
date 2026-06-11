#include "workflow/WorkflowSerializer.h"

#include "nodes/ImageNode.h"
#include "nodes/NodeFactory.h"
#include "util/PathUtils.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Result<WorkflowGraph> WorkflowSerializer::loadFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return Result<WorkflowGraph>::fail(QString("无法打开 workflow：%1").arg(path));
    }
    return fromJson(file.readAll(), path);
}

Status WorkflowSerializer::saveFile(const WorkflowGraph& graph, const QString& path) const
{
    auto json = toJson(graph, path);
    if (json.isFail()) {
        return Status::fail(json.error());
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return Status::fail(QString("无法写入 workflow：%1").arg(path));
    }
    file.write(json.value());
    return Status::ok();
}

Result<WorkflowGraph> WorkflowSerializer::fromJson(const QByteArray& bytes, const QString& workflowFile) const
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return Result<WorkflowGraph>::fail(QString("JSON 解析失败：%1 at %2").arg(parseError.errorString()).arg(parseError.offset));
    }
    if (!doc.isObject()) {
        return Result<WorkflowGraph>::fail("workflow JSON 顶层必须是对象");
    }
    QJsonObject root = doc.object();
    if (root.value("formatVersion").toInt() != 1) {
        return Result<WorkflowGraph>::fail("不支持的 workflow formatVersion");
    }

    WorkflowGraph graph;
    auto& factory = NodeFactory::instance();
    factory.registerBuiltins();

    const QJsonArray nodes = root.value("nodes").toArray();
    for (const auto& nodeValue : nodes) {
        if (!nodeValue.isObject()) return Result<WorkflowGraph>::fail("nodes 数组元素必须是对象");
        const QJsonObject n = nodeValue.toObject();
        const QString id = n.value("id").toString();
        const QString type = n.value("type").toString();
        auto created = factory.create(type);
        if (created.isFail()) return Result<WorkflowGraph>::fail(created.error());
        QJsonObject params = n.value("params").toObject();
        for (const QString& key : {"filePath", "outputPath", "dirPath", "outputDir"}) {
            if (params.contains(key) && params.value(key).isString()) {
                params.insert(key, PathUtils::resolveAgainstFile(params.value(key).toString(), workflowFile));
            }
        }
        auto loaded = created.value()->loadParams(params);
        if (loaded.isFail()) return Result<WorkflowGraph>::fail(QString("节点 %1 参数错误：%2").arg(id, loaded.error()));
        QPointF pos(n.value("x").toDouble(), n.value("y").toDouble());
        auto added = graph.addNodeWithId(id, created.value(), pos);
        if (added.isFail()) return Result<WorkflowGraph>::fail(added.error());
    }

    const QJsonArray edges = root.value("edges").toArray();
    for (const auto& edgeValue : edges) {
        if (!edgeValue.isObject()) return Result<WorkflowGraph>::fail("edges 数组元素必须是对象");
        const QJsonObject e = edgeValue.toObject();
        graph.addEdge(Edge{e.value("fromNode").toString(), e.value("fromPort").toString(),
                           e.value("toNode").toString(), e.value("toPort").toString()});
    }
    return Result<WorkflowGraph>::ok(graph);
}

Result<QByteArray> WorkflowSerializer::toJson(const WorkflowGraph& graph, const QString& workflowFile) const
{
    QJsonObject root;
    root.insert("formatVersion", 1);
    QJsonArray nodes;
    for (const auto& record : graph.nodes()) {
        QJsonObject n;
        n.insert("id", record.id);
        n.insert("type", record.node->typeName());
        n.insert("x", record.position.x());
        n.insert("y", record.position.y());
        QJsonObject params = record.node->saveParams();
        for (const QString& key : {"filePath", "outputPath", "dirPath", "outputDir"}) {
            if (params.contains(key) && params.value(key).isString()) {
                params.insert(key, PathUtils::relativeToFile(params.value(key).toString(), workflowFile));
            }
        }
        n.insert("params", params);
        nodes.append(n);
    }
    root.insert("nodes", nodes);

    QJsonArray edges;
    for (const auto& edge : graph.edges()) {
        QJsonObject e;
        e.insert("fromNode", edge.fromNode);
        e.insert("fromPort", edge.fromPort);
        e.insert("toNode", edge.toNode);
        e.insert("toPort", edge.toPort);
        edges.append(e);
    }
    root.insert("edges", edges);
    return Result<QByteArray>::ok(QJsonDocument(root).toJson(QJsonDocument::Indented));
}
