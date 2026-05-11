#pragma once

#include "core/NodeData.h"
#include "core/Result.h"

#include <QMap>
#include <QStringList>

class WorkflowGraph;

struct ExecutionResult {
    QMap<QString, QMap<QString, NodeData>> nodeOutputs;
    QStringList log;
};

class ExecutionEngine {
public:
    Result<ExecutionResult> execute(const WorkflowGraph& graph) const;
};
