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
    Light,
    Dark,
    System
};

struct Metrics {
    int toolbarIcon = 18;
    int toolbarButton = 28;
    int canvasZoomButtonW = 38;
    int canvasZoomButtonH = 32;
    int canvasZoomOverlayW = 56;
    int canvasZoomOverlayH = 84;
    int paletteMinWidth = 210;
    int propertyMinWidth = 300;
    int previewMinHeight = 230;
    int logMaxHeight = 170;
    int formSpacing = 10;
    int formMargin = 14;
    int dockTitleHeight = 34;
};

struct NodeMetrics {
    double width = 210.0;
    double headerHeight = 36.0;
    double topPadding = 48.0;
    double rowHeight = 24.0;
    double bottomPadding = 18.0;
    double portRadius = 6.0;
    double cornerRadius = 14.0;
    double titleSize = 11.0;
    double labelSize = 10.0;
};

struct Colors {
    QColor canvasTop = QColor("#f8fbff");
    QColor canvasBottom = QColor("#edf4ff");
    QColor canvasDot = QColor(112, 138, 168, 38);
    QColor nodeTop = QColor(255, 255, 255, 232);
    QColor nodeBottom = QColor(232, 240, 252, 226);
    QColor nodeBorder = QColor(126, 154, 192, 120);
    QColor nodeSelected = QColor("#0a84ff");
    QColor nodeShadow = QColor(54, 78, 116, 55);
    QColor textPrimary = QColor("#1f2937");
    QColor textSecondary = QColor("#506174");
    QColor inputPort = QColor("#34c759");
    QColor outputPort = QColor("#0a84ff");
    QColor edge = QColor("#5a7da8");
    QColor edgeSelected = QColor("#ff9f0a");
    QColor pendingEdge = QColor("#0a84ff");
};

Metrics metrics(double uiScale);
NodeMetrics nodeMetrics(double uiScale);
Colors colors();
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
