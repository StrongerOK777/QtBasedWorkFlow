#pragma once

#include <QString>

// 把节点显示名与内部 ID 组合成界面用的中文标签。
// 内部 ID 形如 "ImageInput_1"，界面统一显示中文名加序号："读入图片 #1"。
// 序号取 ID 最后一个下划线之后的部分；没有则只显示中文名。
// 内部 ID 本身不变，仅用于显示，保证 JSON / 存档兼容。
inline QString formatNodeLabel(const QString& displayName, const QString& id)
{
    const QString name = displayName.isEmpty() ? id : displayName;
    const int underscore = id.lastIndexOf(QLatin1Char('_'));
    if (underscore >= 0 && underscore + 1 < id.size()) {
        return QStringLiteral("%1 #%2").arg(name, id.mid(underscore + 1));
    }
    return name;
}
