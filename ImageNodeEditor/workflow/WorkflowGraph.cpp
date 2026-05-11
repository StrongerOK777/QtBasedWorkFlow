#include "workflow/WorkflowGraph.h"

#include "nodes/ImageNode.h"

QString WorkflowGraph::addNode(const QSharedPointer<ImageNode>& node, const QPointF& position)
{
    const QString id = makeNodeId(node ? node->typeName() : "Node");
    addNodeWithId(id, node, position);
    return id;
}

Status WorkflowGraph::addNodeWithId(const QString& id, const QSharedPointer<ImageNode>& node, const QPointF& position)
{
    if (id.isEmpty()) {
        return Status::fail("节点 id 不能为空");
    }
    if (!node) {
        return Status::fail("节点对象为空");
    }
    if (nodes_.contains(id)) {
        return Status::fail(QString("节点 id 重复：%1").arg(id));
    }
    nodes_.insert(id, WorkflowNodeRecord{id, node, position});
    return Status::ok();
}

Status WorkflowGraph::removeNode(const QString& id)
{
    if (!nodes_.remove(id)) {
        return Status::fail(QString("节点不存在：%1").arg(id));
    }
    for (int i = edges_.size() - 1; i >= 0; --i) {
        if (edges_[i].fromNode == id || edges_[i].toNode == id) {
            edges_.removeAt(i);
        }
    }
    return Status::ok();
}

Status WorkflowGraph::addEdge(const Edge& edge)
{
    edges_.append(edge);
    return Status::ok();
}

Status WorkflowGraph::removeEdge(int index)
{
    if (index < 0 || index >= edges_.size()) {
        return Status::fail("连线索引无效");
    }
    edges_.removeAt(index);
    return Status::ok();
}

void WorkflowGraph::clear()
{
    nodes_.clear();
    edges_.clear();
    nextId_ = 1;
}

WorkflowNodeRecord* WorkflowGraph::nodeRecord(const QString& id)
{
    auto it = nodes_.find(id);
    return it == nodes_.end() ? nullptr : &it.value();
}

const WorkflowNodeRecord* WorkflowGraph::nodeRecord(const QString& id) const
{
    auto it = nodes_.constFind(id);
    return it == nodes_.constEnd() ? nullptr : &it.value();
}

QSharedPointer<ImageNode> WorkflowGraph::node(const QString& id) const
{
    const auto* record = nodeRecord(id);
    return record ? record->node : QSharedPointer<ImageNode>{};
}

QVector<WorkflowNodeRecord> WorkflowGraph::nodes() const
{
    QVector<WorkflowNodeRecord> result;
    for (const auto& record : nodes_) {
        result.append(record);
    }
    return result;
}

QStringList WorkflowGraph::nodeIds() const
{
    return nodes_.keys();
}

void WorkflowGraph::setNodePosition(const QString& id, const QPointF& position)
{
    if (auto* record = nodeRecord(id)) {
        record->position = position;
    }
}

QString WorkflowGraph::makeNodeId(const QString& typeName)
{
    QString base = typeName;
    base.remove(' ');
    if (base.isEmpty()) base = "Node";
    QString id;
    do {
        id = QString("%1_%2").arg(base).arg(nextId_++);
    } while (nodes_.contains(id));
    return id;
}
