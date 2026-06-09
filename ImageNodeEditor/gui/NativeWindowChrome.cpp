#include "gui/NativeWindowChrome.h"

#include <QtGlobal>

#ifdef Q_OS_WIN

// Windows 无边框窗口：保留窗口的原生样式（WS_THICKFRAME 等），因此拖边缩放、Aero 贴边、
// 投影和最小化/最大化动画都保持原生行为；只通过 WM_NCCALCSIZE 去掉“可见的”原生标题栏，
// 让程序自带的 QML 标题栏成为唯一的窗口操作区。拖动、最小化、最大化、关闭已由
// WorkbenchBridge -> MainWindow（startSystemMove / showMinimized / showMaximized / close）接好。

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <QAbstractNativeEventFilter>
#include <QCoreApplication>
#include <QSet>
#include <QWidget>

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

namespace {

bool windowIsMaximized(HWND hwnd)
{
    WINDOWPLACEMENT wp{};
    wp.length = sizeof(wp);
    return GetWindowPlacement(hwnd, &wp) && wp.showCmd == SW_SHOWMAXIMIZED;
}

// 当前 DPI 下的窗口外框尺寸（用于最大化时的内缩与边缘缩放抓取带）。
void frameMetrics(HWND hwnd, int& fx, int& fy)
{
    UINT dpi = 96;
    if (auto getDpi = reinterpret_cast<UINT(WINAPI*)(HWND)>(
            ::GetProcAddress(::GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"))) {
        const UINT d = getDpi(hwnd);
        if (d > 0) {
            dpi = d;
        }
    }
    const int padded = ::GetSystemMetrics(SM_CXPADDEDBORDER);
    fx = ::GetSystemMetrics(SM_CXFRAME) + padded;
    fy = ::GetSystemMetrics(SM_CYFRAME) + padded;
    // 按 DPI 粗略缩放，并保证一个可用的最小抓取带。
    fx = (fx * static_cast<int>(dpi)) / 96;
    fy = (fy * static_cast<int>(dpi)) / 96;
    if (fx < 6) {
        fx = 6;
    }
    if (fy < 6) {
        fy = 6;
    }
}

class FramelessFilter : public QAbstractNativeEventFilter {
public:
    QSet<HWND> hwnds;

    bool nativeEventFilter(const QByteArray&, void* message, qintptr* result) override
    {
        auto* msg = static_cast<MSG*>(message);
        if (!msg || !msg->hwnd || !hwnds.contains(msg->hwnd)) {
            return false;
        }
        HWND hwnd = msg->hwnd;

        switch (msg->message) {
        case WM_NCCALCSIZE: {
            // wParam == TRUE 时返回新的客户区。把整窗作为客户区即可去掉标题栏；
            // 最大化时按外框内缩，避免内容被裁切、并让任务栏可见。
            if (msg->wParam == FALSE) {
                return false;
            }
            auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            RECT& rc = params->rgrc[0];
            if (windowIsMaximized(hwnd)) {
                int fx = 0;
                int fy = 0;
                frameMetrics(hwnd, fx, fy);
                rc.left += fx;
                rc.top += fy;
                rc.right -= fx;
                rc.bottom -= fy;
            }
            // 非最大化时保持整窗为客户区（不改 rc），原生标题栏随之消失。
            if (result) {
                *result = 0;
            }
            return true;
        }
        case WM_NCHITTEST: {
            // 最大化时不做边缘缩放；其余交回 Qt（标题栏拖动由 QML 的 startSystemMove 处理）。
            if (windowIsMaximized(hwnd)) {
                if (result) {
                    *result = HTCLIENT;
                }
                return true;
            }
            const int x = GET_X_LPARAM(msg->lParam);
            const int y = GET_Y_LPARAM(msg->lParam);
            RECT w{};
            if (!::GetWindowRect(hwnd, &w)) {
                return false;
            }
            int fx = 0;
            int fy = 0;
            frameMetrics(hwnd, fx, fy);
            const bool left = x < w.left + fx;
            const bool right = x >= w.right - fx;
            const bool top = y < w.top + fy;
            const bool bottom = y >= w.bottom - fy;

            LRESULT hit = HTCLIENT;
            if (top && left) {
                hit = HTTOPLEFT;
            } else if (top && right) {
                hit = HTTOPRIGHT;
            } else if (bottom && left) {
                hit = HTBOTTOMLEFT;
            } else if (bottom && right) {
                hit = HTBOTTOMRIGHT;
            } else if (left) {
                hit = HTLEFT;
            } else if (right) {
                hit = HTRIGHT;
            } else if (top) {
                hit = HTTOP;
            } else if (bottom) {
                hit = HTBOTTOM;
            } else {
                // 客户区内：交回 Qt，让 QML 标题栏接管点击与拖动。
                return false;
            }
            if (result) {
                *result = hit;
            }
            return true;
        }
        default:
            break;
        }
        return false;
    }
};

FramelessFilter* g_filter = nullptr;

}  // namespace

namespace NativeWindowChrome {

void configure(QWidget* window)
{
    if (!window) {
        return;
    }
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return;
    }
    if (!g_filter) {
        g_filter = new FramelessFilter();
        if (auto* app = QCoreApplication::instance()) {
            app->installNativeEventFilter(g_filter);
        }
    }
    g_filter->hwnds.insert(hwnd);

    // 让 DWM 给无边框窗口绘制投影（配合保留的 WS_THICKFRAME）。
    const MARGINS margins{0, 0, 1, 0};
    ::DwmExtendFrameIntoClientArea(hwnd, &margins);

    // 触发一次 WM_NCCALCSIZE，立即应用去标题栏的客户区。
    ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void applyGlobalAppearance()
{
}

}  // namespace NativeWindowChrome

#elif !defined(Q_OS_MACOS)

// 非 Windows、非 macOS（如 Linux）：保持空实现。
namespace NativeWindowChrome {

void configure(QWidget* window)
{
    Q_UNUSED(window);
}

void applyGlobalAppearance()
{
}

}  // namespace NativeWindowChrome

#endif
