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
    Dark
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
    double titleSize = 15.0;
    double labelSize = 13.0;
};

struct Colors {
    QColor canvasTop = QColor("#ffffff");
    QColor canvasBottom = QColor("#f2f2f2");
    QColor canvasDot = QColor(128, 128, 128, 38);
    QColor nodeTop = QColor(255, 255, 255, 232);
    QColor nodeBottom = QColor(232, 232, 232, 226);
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
