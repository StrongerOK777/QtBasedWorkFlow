#include "workflow/ExecutionEngine.h"

#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"
#include "workflow/WorkflowValidator.h"

Result<ExecutionResult> ExecutionEngine::execute(const WorkflowGraph& graph) const
{
    WorkflowValidator validator;
    auto valid = validator.validate(graph);
    if (valid.isFail()) {
        return Result<ExecutionResult>::fail(valid.error());
    }
    auto order = validator.topologicalOrder(graph);
    if (order.isFail()) {
        return Result<ExecutionResult>::fail(order.error());
    }

    ExecutionResult result;
    for (const QString& nodeId : order.value()) {
        auto node = graph.node(nodeId);
        QMap<QString, NodeData> inputs;
        for (const auto& edge : graph.edges()) {
            if (edge.toNode == nodeId) {
                if (!result.nodeOutputs.contains(edge.fromNode) || !result.nodeOutputs[edge.fromNode].contains(edge.fromPort)) {
                    return Result<ExecutionResult>::fail(QString("上游输出不存在：%1.%2").arg(edge.fromNode, edge.fromPort));
                }
                inputs.insert(edge.toPort, result.nodeOutputs[edge.fromNode][edge.fromPort]);
            }
        }
        result.log.append(QString("执行节点 %1 (%2)").arg(nodeId, node->displayName()));
        auto outputs = node->execute(inputs);
        if (outputs.isFail()) {
            result.log.append(QString("节点失败 %1：%2").arg(nodeId, outputs.error()));
            return Result<ExecutionResult>::fail(outputs.error());
        }
        result.nodeOutputs.insert(nodeId, outputs.value());
        result.log.append(QString("节点完成 %1").arg(nodeId));
    }
    return Result<ExecutionResult>::ok(result);
}
