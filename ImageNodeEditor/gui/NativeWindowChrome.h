#pragma once

class QWidget;

namespace NativeWindowChrome {

// 配置窗口原生标题栏：macOS 隐藏标题、透明融合、并按当前主题（AppTheme）设置
// 浅色 / 深色外观与背景色，使浅色主题下不再出现黑色原生标题条。非 macOS 为空实现。
void configure(QWidget* window);

// 设置全局原生外观（macOS：NSApp 的 Aqua / DarkAqua），让所有窗口/对话框/菜单的
// 原生标题栏与红绿灯都跟随当前主题。主题切换后可再次调用。非 macOS 为空实现。
void applyGlobalAppearance();

}
