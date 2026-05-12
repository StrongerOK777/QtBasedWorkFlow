#include "nodes/ImageNode.h"

#include <QJsonValue>

bool ImageNode::isCacheable() const
{
    return true;
}

QVariant ImageNode::parameterValue(const QString& name) const
{
    return saveParams().value(name).toVariant();
}

Status ImageNode::setParameter(const QString& name, const QVariant& value)
{
    QJsonObject object = saveParams();
    object.insert(name, QJsonValue::fromVariant(value));
    return loadParams(object);
}
