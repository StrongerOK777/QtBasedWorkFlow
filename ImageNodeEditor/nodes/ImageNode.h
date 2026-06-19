#pragma once

#include "core/NodeData.h"
#include "core/NodeParameter.h"
#include "core/Result.h"

#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVector>

#include <atomic>
#include <memory>

class ImageNode {
public:
    virtual ~ImageNode() = default;

    virtual QString typeName() const = 0;
    virtual QString displayName() const = 0;
    virtual QVector<PortInfo> inputPorts() const = 0;
    virtual QVector<PortInfo> outputPorts() const = 0;
    virtual QVector<NodeParameter> parameterDefinitions() const = 0;
    virtual QJsonObject saveParams() const = 0;
    virtual Status loadParams(const QJsonObject& object) = 0;
    virtual Result<QMap<QString, NodeData>> execute(const QMap<QString, NodeData>& inputs) = 0;
    virtual bool isCacheable() const;

    virtual QVariant parameterValue(const QString& name) const;
    virtual Status setParameter(const QString& name, const QVariant& value);

    // 执行上下文：引擎在调度本节点前调用，传入本次执行的取消标志。
    // 默认空实现；含内部子图的节点（如宏节点）可保存并传给内部引擎，实现取消传播。
    virtual void onExecutionContext(const std::shared_ptr<std::atomic<bool>>& cancelFlag);
};
