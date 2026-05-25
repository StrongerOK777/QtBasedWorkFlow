#include "gui/NativeWindowChrome.h"

#include <QtGlobal>

#ifdef Q_OS_MACOS

#include <QWidget>

#import <AppKit/AppKit.h>

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

    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.styleMask = nsWindow.styleMask | NSWindowStyleMaskFullSizeContentView;
    nsWindow.backgroundColor = [NSColor colorWithCalibratedWhite:0.12 alpha:1.0];
    nsWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
    nsWindow.title = @"";
    if ([nsWindow respondsToSelector:@selector(setTitlebarSeparatorStyle:)]) {
        nsWindow.titlebarSeparatorStyle = NSTitlebarSeparatorStyleNone;
    }
}

}

#endif
