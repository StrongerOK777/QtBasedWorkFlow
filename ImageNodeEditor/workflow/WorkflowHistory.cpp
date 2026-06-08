#include "workflow/WorkflowHistory.h"

#include "workflow/WorkflowSerializer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace {

// 分组名 → QSettings 组名：timeline → workflowTimeline 等（与既有 GUI 存储一致）。
QString settingsGroup(const QString& group)
{
    if (group.isEmpty()) {
        return QStringLiteral("workflow");
    }
    return QStringLiteral("workflow") + group.left(1).toUpper() + group.mid(1);
}

// 每条快照可能携带的元数据键（用于读取与清理）。
const QStringList& metaKeys()
{
    static const QStringList keys{QStringLiteral("title"), QStringLiteral("label"), QStringLiteral("branch")};
    return keys;
}

}

namespace WorkflowHistory {

QString dataDir(const QString& group)
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(root.isEmpty() ? QDir::homePath() + "/.imagenodeeditor" : root);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    if (!group.isEmpty() && !dir.exists(group)) {
        dir.mkpath(group);
    }
    return group.isEmpty() ? dir.absolutePath() : dir.absoluteFilePath(group);
}

QVector<Entry> list(const QString& group)
{
    QVector<Entry> entries;
    QSettings settings;
    settings.beginGroup(settingsGroup(group));
    const QStringList ids = settings.value("ids").toStringList();
    for (const QString& id : ids) {
        const QString path = settings.value("file/" + id).toString();
        if (path.isEmpty() || !QFileInfo::exists(path)) {
            continue;
        }
        Entry entry;
        entry.id = id;
        entry.filePath = path;
        entry.when = QFileInfo(path).lastModified();
        for (const QString& key : metaKeys()) {
            const QString value = settings.value(key + "/" + id).toString();
            if (!value.isEmpty()) {
                entry.meta.insert(key, value);
            }
        }
        entries.append(entry);
    }
    settings.endGroup();
    return entries;
}

Result<QString> save(const QString& group, const WorkflowGraph& graph,
                     const QMap<QString, QString>& meta, int cap)
{
    const QString id = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    const QString path = QDir(dataDir(group)).absoluteFilePath(id + ".json");
    WorkflowSerializer serializer;
    const Status saved = serializer.saveFile(graph, path);
    if (saved.isFail()) {
        return Result<QString>::fail(saved.error());
    }

    QSettings settings;
    settings.beginGroup(settingsGroup(group));
    QStringList ids = settings.value("ids").toStringList();
    ids.removeAll(id);
    ids.prepend(id);
    if (cap > 0) {
        while (ids.size() > cap) {
            const QString stale = ids.takeLast();
            const QString stalePath = settings.value("file/" + stale).toString();
            if (!stalePath.isEmpty()) {
                QFile::remove(stalePath);
            }
            settings.remove("file/" + stale);
            for (const QString& key : metaKeys()) {
                settings.remove(key + "/" + stale);
            }
        }
    }
    settings.setValue("ids", ids);
    settings.setValue("file/" + id, path);
    for (auto it = meta.cbegin(); it != meta.cend(); ++it) {
        settings.setValue(it.key() + "/" + id, it.value());
    }
    settings.endGroup();
    return Result<QString>::ok(id);
}

Result<WorkflowGraph> load(const QString& group, const QString& id)
{
    QSettings settings;
    settings.beginGroup(settingsGroup(group));
    const QString path = settings.value("file/" + id).toString();
    settings.endGroup();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return Result<WorkflowGraph>::fail(QString("找不到历史记录：%1").arg(id));
    }
    WorkflowSerializer serializer;
    return serializer.loadFile(path);
}

}
