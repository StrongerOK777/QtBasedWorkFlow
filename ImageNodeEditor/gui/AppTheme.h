#pragma once

#include <QColor>
#include <QFont>
#include <QString>

class QApplication;
class QGraphicsDropShadowEffect;
class QMainWindow;
class QWidget;

namespace AppTheme {

constexpr double kMinUiScale = 0.8;
constexpr double kMaxUiScale = 1.8;
constexpr double kUiScaleStep = 0.1;

enum class ThemePreference {
    Dark,
    Light
};

struct Metrics {
    int toolbarIcon = 24;
    int toolbarButton = 30;
    int canvasZoomButtonW = 46;
    int canvasZoomButtonH = 40;
    int canvasZoomOverlayW = 64;
    int canvasZoomOverlayH = 100;
    int paletteMinWidth = 210;
    int propertyMinWidth = 300;
    int previewMinHeight = 230;
    int logMaxHeight = 170;
    int formSpacing = 10;
    int formMargin = 14;
    int dockTitleHeight = 34;
};

struct NodeMetrics {
    double width = 250.0;
    double headerHeight = 42.0;
    double topPadding = 56.0;
    double rowHeight = 30.0;
    double bottomPadding = 22.0;
    double portRadius = 7.5;
    double cornerRadius = 0.0;
    double titleSize = 16.0;
    double labelSize = 14.0;
};

// 主题语义 token：界面所有配色的单一来源。深色 / 浅色各提供一套，
// QSS、QML、画布节点、对话框都从同一套 token 取色，保证整体一致、可切换。
struct Palette {
    // 表面层次
    QColor base;            // 应用主背景
    QColor panel;           // 侧栏 / 标题栏 / 状态栏 / 卡片块
    QColor elevated;        // hover / 浅浮起块
    QColor elevatedHover;   // hover 再强一档
    QColor input;           // 输入框 / 文本域背景
    QColor hairline;        // 细分隔线 / 普通边框
    QColor border;          // 稍强边框
    // 文字
    QColor textPrimary;
    QColor textSecondary;
    QColor textMuted;
    // 强调
    QColor accent;          // 主强调（深色=柔化蓝；浅色=绿）
    QColor accentHover;
    QColor selection;       // 列表 / 菜单选中底色
    QColor selectionText;   // selection 底色上的文字
    QColor onAccent;        // 强调实色（执行按钮等）上的文字
    QColor warning;         // 次强调（浅色=暗黄）
    QColor danger;          // 错误 / 红色反馈
    QColor scrollHandle;
    QColor scrollHandleHover;
    // 画布与节点
    QColor canvasBg;
    QColor canvasDot;
    QColor nodeTop;
    QColor nodeBottom;
    QColor nodeHeader;
    QColor nodeBorder;
    QColor nodeSelected;
    QColor nodeShadow;
    QColor nodeText;
    QColor nodeTextFaded;
    QColor inputPort;
    QColor outputPort;
    QColor edge;
    QColor edgeSelected;
    QColor pendingEdge;
};

struct Colors {
    QColor canvasTop = QColor("#ffffff");
    QColor canvasBottom = QColor("#f2f2f2");
    QColor canvasDot = QColor(128, 128, 128, 38);
    QColor nodeTop = QColor(255, 255, 255, 232);
    QColor nodeBottom = QColor(232, 232, 232, 226);
    QColor nodeHeader = QColor(255, 255, 255, 232);
    QColor nodeBorder = QColor(128, 128, 128, 120);
    QColor nodeSelected = QColor("#111111");
    QColor nodeShadow = QColor(0, 0, 0, 55);
    QColor textPrimary = QColor("#202020");
    QColor textSecondary = QColor("#606060");
    QColor inputPort = QColor("#707070");
    QColor outputPort = QColor("#404040");
    QColor edge = QColor("#707070");
    QColor edgeSelected = QColor("#202020");
    QColor pendingEdge = QColor("#404040");
};

Metrics metrics(double uiScale);
NodeMetrics nodeMetrics(double uiScale);
Colors colors();
Palette palette();
Palette darkPalette();
Palette lightPalette();
QFont appFont(double uiScale);
QString styleSheet(double uiScale);
int px(double value, double uiScale);
double clampedScale(double uiScale);
ThemePreference themePreference();
QString themePreferenceName();
void setThemePreference(const QString& preference);
void setThemePreference(ThemePreference preference);
bool isDarkTheme();

void apply(QApplication& app, double uiScale);
QGraphicsDropShadowEffect* makeShadow(QWidget* parent, double uiScale, int alpha = 48);

}
