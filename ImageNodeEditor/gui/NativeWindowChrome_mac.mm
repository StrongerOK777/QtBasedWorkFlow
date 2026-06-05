#include "gui/NativeWindowChrome.h"

#include <QtGlobal>

#ifdef Q_OS_MACOS

#include "gui/AppTheme.h"

#include <QColor>
#include <QWidget>

#import <AppKit/AppKit.h>

namespace {

NSAppearance* currentAppearance()
{
    return [NSAppearance appearanceNamed:(AppTheme::isDarkTheme() ? NSAppearanceNameDarkAqua
                                                                  : NSAppearanceNameAqua)];
}

}

namespace NativeWindowChrome {

void configure(QWidget* window)
{
    if (!window) {
        return;
    }

    auto* view = reinterpret_cast<NSView*>(window->winId());
    NSWindow* nsWindow = view.window;
    if (!nsWindow) {
        return;
    }

    // 背景与外观跟随当前主题：浅色主题用浅色 base + Aqua，深色主题用深色 base + DarkAqua，
    // 使浅色模式下顶部原生标题栏区域不再是黑条。
    const QColor base = AppTheme::palette().base;
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.styleMask = nsWindow.styleMask | NSWindowStyleMaskFullSizeContentView;
    nsWindow.backgroundColor = [NSColor colorWithSRGBRed:base.redF()
                                                   green:base.greenF()
                                                    blue:base.blueF()
                                                   alpha:1.0];
    nsWindow.appearance = currentAppearance();
    nsWindow.title = @"";
    if ([nsWindow respondsToSelector:@selector(setTitlebarSeparatorStyle:)]) {
        nsWindow.titlebarSeparatorStyle = NSTitlebarSeparatorStyleNone;
    }
}

void applyGlobalAppearance()
{
    // 让所有窗口/对话框/菜单与红绿灯统一跟随当前主题，避免浅色主题下出现深色原生标题栏。
    [NSApplication sharedApplication].appearance = currentAppearance();
}

}

#endif
