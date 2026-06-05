#pragma once

#include <QIcon>
#include <QString>

namespace AppIcon {

// 应用 / 窗口图标。
QIcon makeAppIcon();

// 运行时矢量绘制的线性工具图标（菜单、工具栏、画布按钮统一用它）。颜色随当前
// 主题（AppTheme）深浅自适应。name 见实现中的分支，如 "new"/"open"/"run"/"settings" 等。
QIcon lineIcon(const QString& name);

}
