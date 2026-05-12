#include "nodes/MacroNode.h"

#include "nodes/NodeFactory.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowSerializer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSharedPointer>

namespace {

QJsonObject mappingToJson(const MacroPortMapping& mapping)
{
    QJsonObject object;
    object.insert("macroPort", mapping.macroPort);
    object.insert("displayName", mapping.displayName);
    object.insert("internalNode", mapping.internalNode);
    object.insert("internalPort", mapping.internalPort);
    object.insert("type", portTypeName(mapping.type));
    return object;
}

PortType portTypeFromName(const QString& name)
{
    if (name == "ImageGray") return PortType::ImageGray;
    if (name == "Number") return PortType::Number;
    if (name == "Text") return PortType::Text;
    if (name == "Mask") return PortType::Mask;
    if (name == "ImageList") return PortType::ImageList;
    return PortType::ImageRGBA;
}

Result<MacroPortMapping> mappingFromJson(const QJsonValue& value)
{
    if (!value.isObject()) {
        return Result<MacroPortMapping>::fail("宏节点端口映射必须是对象");
    }
    const QJsonObject object = value.toObject();
    MacroPortMapping mapping;
    mapping.macroPort = object.value("macroPort").toString();
    mapping.displayName = object.value("displayName").toString(mapping.macroPort);
    mapping.internalNode = object.value("internalNode").toString();
    mapping.internalPort = object.value("internalPort").toString();
    mapping.type = portTypeFromName(object.value("type").toString());
    if (mapping.macroPort.isEmpty() || mapping.internalNode.isEmpty() || mapping.internalPort.isEmpty()) {
        return Result<MacroPortMapping>::fail("宏节点端口映射字段不完整");
    }
    return Result<MacroPortMapping>::ok(mapping);
}

QJsonArray mappingsToJson(const QVector<MacroPortMapping>& mappings)
{
    QJsonArray array;
    for (const auto& mapping : mappings) {
        array.append(mappingToJson(mapping));
    }
    return array;
}

Result<QVector<MacroPortMapping>> mappingsFromJson(const QJsonValue& value)
{
    if (!value.isArray()) {
        return Result<QVector<MacroPortMapping>>::fail("宏节点端口映射必须是数组");
    }
    QVector<MacroPortMapping> mappings;
    for (const auto& item : value.toArray()) {
        auto mapping = mappingFromJson(item);
        if (mapping.isFail()) {
            return Result<QVector<MacroPortMapping>>::fail(mapping.error());
        }
        mappings.append(mapping.value());
    }
    return Result<QVector<MacroPortMapping>>::ok(mappings);
}

}

QString MacroNode::typeName() const
{
    return "Macro";
}

QString MacroNode::displayName() const
{
    return displayName_;
}

QVector<PortInfo> MacroNode::inputPorts() const
{
    QVector<PortInfo> ports;
    for (const auto& mapping : inputMappings_) {
        ports.append(PortInfo{mapping.macroPort, mapping.displayName, PortDirection::Input, mapping.type, true, false});
    }
    return ports;
}

QVector<PortInfo> MacroNode::outputPorts() const
{
    QVector<PortInfo> ports;
    for (const auto& mapping : outputMappings_) {
        ports.append(PortInfo{mapping.macroPort, mapping.displayName, PortDirection::Output, mapping.type, true, true});
    }
    return ports;
}

QVector<NodeParameter> MacroNode::parameterDefinitions() const
{
    return {NodeParameter{"displayName", "名称", ParameterType::Text, displayName_, 0.0, 0.0, {}, false}};
}

QJsonObject MacroNode::saveParams() const
{
    QJsonObject object;
    object.insert("displayName", displayName_);
    object.insert("inputs", mappingsToJson(inputMappings_));
    object.insert("outputs", mappingsToJson(outputMappings_));
    WorkflowSerializer serializer;
    auto graphJson = serializer.toJson(subgraph_);
    if (graphJson.isOk()) {
        const QJsonDocument doc = QJsonDocument::fromJson(graphJson.value());
        if (doc.isObject()) {
            object.insert("graph", doc.object());
        }
    }
    return object;
}

Status MacroNode::loadParams(const QJsonObject& object)
{
    displayName_ = object.value("displayName").toString("宏节点");

    auto inputs = mappingsFromJson(object.value("inputs"));
    if (inputs.isFail()) {
        return Status::fail(inputs.error());
    }
    auto outputs = mappingsFromJson(object.value("outputs"));
    if (outputs.isFail()) {
        return Status::fail(outputs.error());
    }

    WorkflowGraph loadedGraph;
    if (object.contains("graph")) {
        if (!object.value("graph").isObject()) {
            return Status::fail("宏节点 graph 必须是对象");
        }
        WorkflowSerializer serializer;
        const QByteArray bytes = QJsonDocument(object.value("graph").toObject()).toJson(QJsonDocument::Compact);
        auto graph = serializer.fromJson(bytes);
        if (graph.isFail()) {
            return Status::fail(QString("宏节点子图加载失败：%1").arg(graph.error()));
        }
        loadedGraph = graph.value();
    }

    inputMappings_ = inputs.value();
    outputMappings_ = outputs.value();
    subgraph_ = loadedGraph;
    return Status::ok();
}

Result<QMap<QString, NodeData>> MacroNode::execute(const QMap<QString, NodeData>& inputs)
{
    ExecutionEngine engine;
    ExecutionEngine::ExternalInputMap externalInputs;
    for (const auto& mapping : inputMappings_) {
        if (!inputs.contains(mapping.macroPort)) {
            return Result<QMap<QString, NodeData>>::fail(QString("宏节点缺少输入：%1").arg(mapping.displayName));
        }
        externalInputs.insert(QString("%1.%2").arg(mapping.internalNode, mapping.internalPort), inputs.value(mapping.macroPort));
    }

    auto result = engine.executeWithExternalInputs(subgraph_, externalInputs);
    if (result.isFail()) {
        return Result<QMap<QString, NodeData>>::fail(result.error());
    }

    QMap<QString, NodeData> outputs;
    for (const auto& mapping : outputMappings_) {
        const auto nodeOutputs = result.value().nodeOutputs.value(mapping.internalNode);
        if (!nodeOutputs.contains(mapping.internalPort)) {
            return Result<QMap<QString, NodeData>>::fail(QString("宏节点内部输出不存在：%1.%2").arg(mapping.internalNode, mapping.internalPort));
        }
        outputs.insert(mapping.macroPort, nodeOutputs.value(mapping.internalPort));
    }
    return Result<QMap<QString, NodeData>>::ok(outputs);
}

QVariant MacroNode::parameterValue(const QString& name) const
{
    if (name == "displayName") {
        return displayName_;
    }
    return {};
}

Status MacroNode::setParameter(const QString& name, const QVariant& value)
{
    if (name == "displayName") {
        displayName_ = value.toString().trimmed().isEmpty() ? "宏节点" : value.toString().trimmed();
        return Status::ok();
    }
    return Status::fail(QString("未知参数：%1").arg(name));
}

void MacroNode::setSubgraph(const WorkflowGraph& graph)
{
    subgraph_ = graph;
}

void MacroNode::setInputMappings(const QVector<MacroPortMapping>& mappings)
{
    inputMappings_ = mappings;
}

void MacroNode::setOutputMappings(const QVector<MacroPortMapping>& mappings)
{
    outputMappings_ = mappings;
}

void MacroNode::setDisplayName(const QString& name)
{
    displayName_ = name.trimmed().isEmpty() ? "宏节点" : name.trimmed();
}

void registerMacroNode(NodeFactory& factory)
{
    factory.registerNode("Macro", "宏节点", "高级功能", [] {
        return QSharedPointer<ImageNode>(new MacroNode);
    });
}
