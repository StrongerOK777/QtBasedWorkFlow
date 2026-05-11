#pragma once

#include "core/NodeData.h"
#include "core/NodeParameter.h"
#include "core/Result.h"

#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVector>

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

    virtual QVariant parameterValue(const QString& name) const;
    virtual Status setParameter(const QString& name, const QVariant& value);
};
