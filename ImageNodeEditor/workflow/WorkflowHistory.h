#pragma once

#include "core/Result.h"
#include "workflow/WorkflowGraph.h"

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVector>

// 工作流保存历史的共享存储（GUI 与命令行共用同一套，互相可见）。
// 快照 JSON 存于 AppDataLocation/<group>/<id>.json；元数据存于 QSettings 组
// "workflow<Group>"（如 group="checkpoints" → 组 "workflowCheckpoints"），键为
// ids / file/<id> / title|label|branch/<id>。保持与既有 GUI 存储格式一致，保证互通。
namespace WorkflowHistory {

// 常用分组名（与 GUI 既有存储一致）。
inline const QString kTimeline = QStringLiteral("timeline");        // 每次保存的自动时间线
inline const QString kCheckpoints = QStringLiteral("checkpoints");  // 手动保存点
inline const QString kTemplates = QStringLiteral("templates");      // 方案库模板

struct Entry {
    QString id;
    QString filePath;
    QDateTime when;                 // 取快照文件最后修改时间
    QMap<QString, QString> meta;    // title / label / branch 等原始元数据
};

// 返回 <group> 快照目录（必要时创建）。
QString dataDir(const QString& group);

// 列出某分组的历史，newest-first（按 QSettings 中 ids 顺序）。
QVector<Entry> list(const QString& group);

// 保存图的快照到某分组，写入 meta（如 {"title": ...}）。cap>0 时只保留最近 cap 条，
// 多余的连同快照文件一起删除。返回新快照 id。
Result<QString> save(const QString& group, const WorkflowGraph& graph,
                     const QMap<QString, QString>& meta, int cap = 0);

// 按 id 载入某分组的快照为图。
Result<WorkflowGraph> load(const QString& group, const QString& id);

}
