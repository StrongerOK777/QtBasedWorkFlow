#pragma once

#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"

class NodeFactory;

struct MacroPortMapping {
    QString macroPort;
    QString displayName;
    QString internalNode;
    QString internalPort;
    PortType type = PortType::ImageRGBA;
};

class MacroNode final : public ImageNode {
public:
    QString typeName() const override;
    QString displayName() const override;
    QVector<PortInfo> inputPorts() const override;
    QVector<PortInfo> outputPorts() const override;
    QVector<NodeParameter> parameterDefinitions() const override;
    QJsonObject saveParams() const override;
    Status loadParams(const QJsonObject& object) override;
    Result<QMap<QString, NodeData>> execute(const QMap<QString, NodeData>& inputs) override;

    QVariant parameterValue(const QString& name) const override;
    Status setParameter(const QString& name, const QVariant& value) override;

    const WorkflowGraph& subgraph() const { return subgraph_; }
    WorkflowGraph& subgraph() { return subgraph_; }
    QVector<MacroPortMapping> inputMappings() const { return inputMappings_; }
    QVector<MacroPortMapping> outputMappings() const { return outputMappings_; }

    void setSubgraph(const WorkflowGraph& graph);
    void setInputMappings(const QVector<MacroPortMapping>& mappings);
    void setOutputMappings(const QVector<MacroPortMapping>& mappings);
    void setDisplayName(const QString& name);

private:
    QString displayName_ = "宏节点";
    WorkflowGraph subgraph_;
    QVector<MacroPortMapping> inputMappings_;
    QVector<MacroPortMapping> outputMappings_;
};

void registerMacroNode(NodeFactory& factory);
