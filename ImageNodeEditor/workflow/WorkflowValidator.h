#pragma once

#include "core/Edge.h"
#include "core/Result.h"

#include <QStringList>
#include <QVector>

class WorkflowGraph;

class WorkflowValidator {
public:
    Status validate(const WorkflowGraph& graph) const;
    Status validateEdge(const WorkflowGraph& graph, const Edge& edge) const;
    Result<QStringList> topologicalOrder(const WorkflowGraph& graph) const;
};
