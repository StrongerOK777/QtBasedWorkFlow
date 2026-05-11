#include "nodes/NodeFactory.h"

#include "nodes/BasicNodes.h"
#include "nodes/ImageNode.h"

#include <QMap>

NodeFactory& NodeFactory::instance()
{
    static NodeFactory factory;
    return factory;
}

void NodeFactory::registerNode(const QString& typeName, const QString& displayName, const QString& category, Creator creator)
{
    entries_.insert(typeName, Entry{NodeDescriptor{typeName, displayName, category}, std::move(creator)});
}

Result<QSharedPointer<ImageNode>> NodeFactory::create(const QString& typeName) const
{
    auto it = entries_.find(typeName);
    if (it == entries_.end()) {
        return Result<QSharedPointer<ImageNode>>::fail(QString("未知节点类型：%1").arg(typeName));
    }
    return Result<QSharedPointer<ImageNode>>::ok(it.value().creator());
}

QVector<NodeDescriptor> NodeFactory::descriptors() const
{
    QVector<NodeDescriptor> result;
    for (const auto& entry : entries_) {
        result.append(entry.descriptor);
    }
    std::sort(result.begin(), result.end(), [](const NodeDescriptor& a, const NodeDescriptor& b) {
        if (a.category == b.category) return a.displayName < b.displayName;
        return a.category < b.category;
    });
    return result;
}

QStringList NodeFactory::typeNames() const
{
    return entries_.keys();
}

void NodeFactory::registerBuiltins()
{
    if (builtinsRegistered_) {
        return;
    }
    builtinsRegistered_ = true;
    registerBasicNodes(*this);
}
