#pragma once

#include "core/Result.h"

#include <QSharedPointer>
#include <QMap>
#include <QStringList>
#include <QVector>
#include <functional>

class ImageNode;

struct NodeDescriptor {
    QString typeName;
    QString displayName;
    QString category;
};

class NodeFactory {
public:
    using Creator = std::function<QSharedPointer<ImageNode>()>;

    static NodeFactory& instance();

    void registerNode(const QString& typeName, const QString& displayName, const QString& category, Creator creator);
    Result<QSharedPointer<ImageNode>> create(const QString& typeName) const;
    QVector<NodeDescriptor> descriptors() const;
    QStringList typeNames() const;
    void registerBuiltins();

private:
    struct Entry {
        NodeDescriptor descriptor;
        Creator creator;
    };

    QMap<QString, Entry> entries_;
    bool builtinsRegistered_ = false;
};
