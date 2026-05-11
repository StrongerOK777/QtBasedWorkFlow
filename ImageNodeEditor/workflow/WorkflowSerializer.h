#pragma once

#include "core/Result.h"
#include "workflow/WorkflowGraph.h"

#include <QString>

class WorkflowSerializer {
public:
    Result<WorkflowGraph> loadFile(const QString& path) const;
    Status saveFile(const WorkflowGraph& graph, const QString& path) const;
    Result<WorkflowGraph> fromJson(const QByteArray& bytes, const QString& workflowFile = {}) const;
    Result<QByteArray> toJson(const WorkflowGraph& graph, const QString& workflowFile = {}) const;
};
