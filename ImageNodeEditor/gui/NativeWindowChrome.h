#pragma once

class QWidget;

namespace NativeWindowChrome {

void configure(QWidget* window);

// 设置全局深色外观（macOS：NSApp 深色 Aqua），使所有窗口/对话框/菜单的
// 原生标题栏与红绿灯都为深色，与深色主题一致。非 macOS 为空实现。
void applyGlobalDarkAppearance();

}
