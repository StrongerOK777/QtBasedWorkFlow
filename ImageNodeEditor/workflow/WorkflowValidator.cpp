#include "workflow/WorkflowValidator.h"

#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"

#include <QMap>
#include <QQueue>
#include <QSet>

namespace {

const PortInfo* findPort(const QVector<PortInfo>& ports, const QString& name)
{
    for (const auto& port : ports) {
        if (port.name == name) return &port;
    }
    return nullptr;
}

}

Status WorkflowValidator::validateEdge(const WorkflowGraph& graph, const Edge& edge) const
{
    auto from = graph.node(edge.fromNode);
    auto to = graph.node(edge.toNode);
    if (!from) return Status::fail(QString("连线起点节点不存在：%1").arg(edge.fromNode));
    if (!to) return Status::fail(QString("连线终点节点不存在：%1").arg(edge.toNode));
    if (edge.fromNode == edge.toNode) return Status::fail("不允许节点连接到自身");

    const PortInfo* fromPort = findPort(from->outputPorts(), edge.fromPort);
    const PortInfo* toPort = findPort(to->inputPorts(), edge.toPort);
    if (!fromPort) return Status::fail(QString("输出端口不存在：%1.%2").arg(edge.fromNode, edge.fromPort));
    if (!toPort) return Status::fail(QString("输入端口不存在：%1.%2").arg(edge.toNode, edge.toPort));
    if (!portTypesCompatible(fromPort->type, toPort->type)) {
        return Status::fail(QString("端口类型不兼容：%1 -> %2").arg(portTypeName(fromPort->type), portTypeName(toPort->type)));
    }

    if (!toPort->allowMultipleConnections) {
        for (const auto& existing : graph.edges()) {
            if (existing.toNode == edge.toNode && existing.toPort == edge.toPort) {
                return Status::fail(QString("输入端口只能连接一次：%1.%2").arg(edge.toNode, edge.toPort));
            }
        }
    }

    WorkflowGraph copy = graph;
    copy.addEdge(edge);
    auto order = topologicalOrder(copy);
    if (order.isFail()) {
        return Status::fail(order.error());
    }
    return Status::ok();
}

Status WorkflowValidator::validate(const WorkflowGraph& graph) const
{
    QSet<QString> seenInputs;
    for (const auto& edge : graph.edges()) {
        auto from = graph.node(edge.fromNode);
        auto to = graph.node(edge.toNode);
        if (!from) return Status::fail(QString("连线起点节点不存在：%1").arg(edge.fromNode));
        if (!to) return Status::fail(QString("连线终点节点不存在：%1").arg(edge.toNode));
        if (edge.fromNode == edge.toNode) return Status::fail("不允许节点连接到自身");
        const PortInfo* fromPort = findPort(from->outputPorts(), edge.fromPort);
        const PortInfo* toPort = findPort(to->inputPorts(), edge.toPort);
        if (!fromPort) return Status::fail(QString("输出端口不存在：%1.%2").arg(edge.fromNode, edge.fromPort));
        if (!toPort) return Status::fail(QString("输入端口不存在：%1.%2").arg(edge.toNode, edge.toPort));
        if (!portTypesCompatible(fromPort->type, toPort->type)) {
            return Status::fail(QString("端口类型不兼容：%1 -> %2").arg(portTypeName(fromPort->type), portTypeName(toPort->type)));
        }
        const QString key = edge.toNode + "." + edge.toPort;
        if (toPort && !toPort->allowMultipleConnections && seenInputs.contains(key)) {
            return Status::fail(QString("输入端口被多次连接：%1").arg(key));
        }
        seenInputs.insert(key);
    }
    for (const auto& record : graph.nodes()) {
        for (const auto& port : record.node->inputPorts()) {
            if (port.required && !seenInputs.contains(record.id + "." + port.name)) {
                return Status::fail(QString("节点 %1 缺少必需输入：%2").arg(record.id, port.displayName));
            }
        }
    }
    auto order = topologicalOrder(graph);
    if (order.isFail()) {
        return Status::fail(order.error());
    }
    return Status::ok();
}

Result<QStringList> WorkflowValidator::topologicalOrder(const WorkflowGraph& graph) const
{
    QMap<QString, int> indegree;
    QMap<QString, QStringList> next;
    for (const QString& id : graph.nodeIds()) {
        indegree[id] = 0;
    }
    for (const auto& edge : graph.edges()) {
        if (!indegree.contains(edge.fromNode) || !indegree.contains(edge.toNode)) {
            return Result<QStringList>::fail("拓扑排序失败：连线引用了不存在的节点");
        }
        ++indegree[edge.toNode];
        next[edge.fromNode].append(edge.toNode);
    }

    QQueue<QString> queue;
    for (auto it = indegree.cbegin(); it != indegree.cend(); ++it) {
        if (it.value() == 0) queue.enqueue(it.key());
    }

    QStringList order;
    while (!queue.isEmpty()) {
        const QString id = queue.dequeue();
        order.append(id);
        for (const QString& downstream : next.value(id)) {
            --indegree[downstream];
            if (indegree[downstream] == 0) queue.enqueue(downstream);
        }
    }

    if (order.size() != indegree.size()) {
        return Result<QStringList>::fail("工作流包含环，无法执行");
    }
    return Result<QStringList>::ok(order);
}
