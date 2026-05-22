#pragma once

#include "workflow/ExecutionEngine.h"

#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>

#include <QJsonObject>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <functional>
#include <memory>

class ImageNode;
class QWidget;

class WorkflowNodeDelegate final : public QtNodes::NodeDelegateModel {
public:
    using ParameterChanged = std::function<void(const QString&, const QString&, const QVariant&)>;

    WorkflowNodeDelegate(QString workflowNodeId,
                         QString category,
                         QSharedPointer<ImageNode> node,
                         double uiScale,
                         ParameterChanged parameterChanged);

    QString workflowNodeId() const { return workflowNodeId_; }
    QString category() const { return category_; }
    QSharedPointer<ImageNode> imageNode() const { return node_; }
    NodeExecutionState executionState() const { return executionState_; }
    qint64 elapsedMs() const { return elapsedMs_; }
    int animationPhase() const { return animationPhase_; }

    QString name() const override;
    QString caption() const override;
    bool captionVisible() const override { return true; }
    QString portCaption(QtNodes::PortType portType, QtNodes::PortIndex index) const override;
    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; }
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex index) const override;
    QtNodes::ConnectionPolicy portConnectionPolicy(QtNodes::PortType portType, QtNodes::PortIndex index) const override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex port) override;
    QWidget* embeddedWidget() override;
    QJsonObject save() const override { return {}; }
    void load(const QJsonObject&) override {}

    void setExpanded(bool expanded);
    void setExecutionState(NodeExecutionState state);
    void setElapsedMs(qint64 elapsedMs);
    void setAnimationPhase(int phase);

private:
    QWidget* buildParameterPanel();
    QWidget* makeParameterEditor(const struct NodeParameter& parameter);

    QString workflowNodeId_;
    QString category_;
    QSharedPointer<ImageNode> node_;
    double uiScale_ = 1.0;
    ParameterChanged parameterChanged_;
    QWidget* parameterPanel_ = nullptr;
    bool expanded_ = false;
    NodeExecutionState executionState_ = NodeExecutionState::NotExecuted;
    qint64 elapsedMs_ = -1;
    int animationPhase_ = 0;
};
