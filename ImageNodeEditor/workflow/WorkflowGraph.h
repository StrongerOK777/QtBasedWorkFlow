#pragma once

#include "core/Edge.h"
#include "core/Result.h"

#include <QMap>
#include <QPointF>
#include <QSharedPointer>
#include <QString>
#include <QVector>

class ImageNode;

struct WorkflowNodeRecord {
    QString id;
    QSharedPointer<ImageNode> node;
    QPointF position;
};

class WorkflowGraph {
public:
    QString addNode(const QSharedPointer<ImageNode>& node, const QPointF& position = {});
    Status addNodeWithId(const QString& id, const QSharedPointer<ImageNode>& node, const QPointF& position = {});
    Status removeNode(const QString& id);
    Status addEdge(const Edge& edge);
    Status removeEdge(int index);
    void clear();

    WorkflowNodeRecord* nodeRecord(const QString& id);
    const WorkflowNodeRecord* nodeRecord(const QString& id) const;
    QSharedPointer<ImageNode> node(const QString& id) const;
    QVector<WorkflowNodeRecord> nodes() const;
    const QVector<Edge>& edges() const { return edges_; }
    QVector<Edge>& edges() { return edges_; }
    QStringList nodeIds() const;
    bool containsNode(const QString& id) const { return nodes_.contains(id); }
    void setNodePosition(const QString& id, const QPointF& position);

private:
    QString makeNodeId(const QString& typeName);

    QMap<QString, WorkflowNodeRecord> nodes_;
    QVector<Edge> edges_;
    int nextId_ = 1;
};
