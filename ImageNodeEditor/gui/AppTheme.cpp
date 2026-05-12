#include "gui/AppTheme.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace {

AppTheme::ThemePreference g_themePreference = AppTheme::ThemePreference::System;

QString pxValue(double value, double scale)
{
    return QString::number(AppTheme::px(value, scale));
}

}

namespace AppTheme {

double clampedScale(double uiScale)
{
    return std::clamp(uiScale, kMinUiScale, kMaxUiScale);
}

int px(double value, double uiScale)
{
    return std::max(1, int(std::lround(value * clampedScale(uiScale))));
}

Metrics metrics(double uiScale)
{
    Metrics m;
    m.toolbarIcon = px(18, uiScale);
    m.toolbarButton = px(28, uiScale);
    m.canvasZoomButtonW = px(38, uiScale);
    m.canvasZoomButtonH = px(32, uiScale);
    m.canvasZoomOverlayW = px(56, uiScale);
    m.canvasZoomOverlayH = px(84, uiScale);
    m.paletteMinWidth = px(210, uiScale);
    m.propertyMinWidth = px(300, uiScale);
    m.previewMinHeight = px(230, uiScale);
    m.logMaxHeight = px(170, uiScale);
    m.formSpacing = px(10, uiScale);
    m.formMargin = px(14, uiScale);
    m.dockTitleHeight = px(34, uiScale);
    return m;
}

NodeMetrics nodeMetrics(double uiScale)
{
    const double s = clampedScale(uiScale);
    NodeMetrics m;
    m.width = 210.0 * s;
    m.headerHeight = 36.0 * s;
    m.topPadding = 48.0 * s;
    m.rowHeight = 24.0 * s;
    m.bottomPadding = 18.0 * s;
    m.portRadius = 6.0 * s;
    m.cornerRadius = 14.0 * s;
    m.titleSize = 11.0 * s;
    m.labelSize = 10.0 * s;
    return m;
}

Colors colors()
{
    if (!isDarkTheme()) {
        return {};
    }
    Colors c;
    c.canvasTop = QColor("#111827");
    c.canvasBottom = QColor("#0b1220");
    c.canvasDot = QColor(148, 163, 184, 36);
    c.nodeTop = QColor(35, 45, 63, 238);
    c.nodeBottom = QColor(21, 29, 43, 232);
    c.nodeBorder = QColor(99, 130, 170, 130);
    c.nodeSelected = QColor("#60a5fa");
    c.nodeShadow = QColor(0, 0, 0, 90);
    c.textPrimary = QColor("#e5edf7");
    c.textSecondary = QColor("#aab7c8");
    c.inputPort = QColor("#30d158");
    c.outputPort = QColor("#60a5fa");
    c.edge = QColor("#8fb8e8");
    c.edgeSelected = QColor("#ffb340");
    c.pendingEdge = QColor("#60a5fa");
    return c;
}

QFont appFont(double uiScale)
{
    QFont font = QApplication::font();
    font.setPointSizeF(std::max(9.0, 11.0 * clampedScale(uiScale)));
    return font;
}

QString styleSheet(double uiScale)
{
    const int radius = px(14, uiScale);
    const int smallRadius = px(9, uiScale);
    const int padV = px(6, uiScale);
    const int padH = px(10, uiScale);
    const int itemHeight = px(32, uiScale);
    const int titleHeight = px(34, uiScale);
    const int border = std::max(1, px(1, uiScale));

    QString sheet = QString(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #fbfdff, stop:0.55 #f2f7ff, stop:1 #e8f1ff);
        }
        QMenuBar {
            background: rgba(255, 255, 255, 212);
            border-bottom: %1px solid rgba(142, 160, 186, 90);
            padding: %2px %3px;
            color: #1f2937;
        }
        QMenuBar::item {
            border-radius: %4px;
            padding: %2px %5px;
            background: transparent;
        }
        QMenuBar::item:selected {
            background: rgba(10, 132, 255, 34);
        }
        QMenu {
            background: rgba(255, 255, 255, 238);
            border: %1px solid rgba(120, 145, 180, 90);
            border-radius: %4px;
            padding: %2px;
            color: #1f2937;
        }
        QMenu::item {
            min-height: %6px;
            border-radius: %4px;
            padding: %2px %7px;
        }
        QMenu::item:selected {
            background: rgba(10, 132, 255, 42);
            color: #0b3d72;
        }
        QToolBar {
            background: rgba(255, 255, 255, 178);
            border: %1px solid rgba(255, 255, 255, 180);
            border-bottom-color: rgba(120, 145, 180, 82);
            spacing: %1px;
            padding: %1px %2px;
            movable: false;
        }
        QToolBar QToolButton {
            min-width: %12px;
            min-height: %12px;
            padding: %1px;
            border: %1px solid transparent;
            border-radius: %4px;
            background: transparent;
        }
        QToolButton, QPushButton {
            min-height: %6px;
            border-radius: %4px;
            padding: %2px %5px;
            border: %1px solid rgba(105, 130, 166, 88);
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,238), stop:1 rgba(228,238,252,220));
            color: #1f2937;
        }
        QToolBar QToolButton:hover {
            border-color: rgba(10, 132, 255, 96);
            background: rgba(10, 132, 255, 28);
        }
        QToolBar QToolButton:pressed {
            background: rgba(10, 132, 255, 52);
            padding-top: %1px;
            padding-bottom: %1px;
        }
        QToolButton:hover, QPushButton:hover {
            border-color: rgba(10, 132, 255, 148);
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,250), stop:1 rgba(218,235,255,232));
        }
        QToolButton:pressed, QPushButton:pressed {
            background: rgba(190, 217, 249, 232);
            padding-top: %8px;
            padding-bottom: %9px;
        }
        QDockWidget {
            color: #233044;
            titlebar-close-icon: none;
            titlebar-normal-icon: none;
        }
        QDockWidget::title {
            min-height: %10px;
            padding-left: %7px;
            text-align: left;
            background: rgba(255, 255, 255, 196);
            border: %1px solid rgba(255, 255, 255, 188);
            border-bottom-color: rgba(126, 154, 192, 78);
            border-top-left-radius: %4px;
            border-top-right-radius: %4px;
        }
        QDockWidget > QWidget, QWidget#glassPanel {
            background: rgba(255, 255, 255, 174);
            border: %1px solid rgba(255, 255, 255, 168);
            border-radius: %11px;
        }
        QWidget#canvasZoomOverlay {
            background: rgba(255, 255, 255, 178);
            border: %1px solid rgba(126, 154, 192, 86);
            border-radius: %4px;
        }
        QLabel#previewPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255, 255, 255, 220), stop:1 rgba(232, 241, 255, 210));
            border: %1px solid rgba(126, 154, 192, 88);
            border-radius: %11px;
            color: #526174;
        }
        QTextEdit#logPanel {
            background: rgba(255, 255, 255, 188);
            border-radius: %11px;
        }
        QListWidget, QTextEdit, QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            border: %1px solid rgba(126, 154, 192, 94);
            border-radius: %4px;
            background: rgba(255, 255, 255, 214);
            selection-background-color: rgba(10, 132, 255, 56);
            selection-color: #0b3d72;
            color: #1f2937;
        }
        QListWidget {
            padding: %2px;
            outline: 0;
        }
        QListWidget::item {
            min-height: %6px;
            border-radius: %4px;
            padding: %2px %5px;
        }
        QListWidget::item:hover {
            background: rgba(10, 132, 255, 28);
        }
        QListWidget::item:selected {
            background: rgba(10, 132, 255, 58);
            color: #0b3d72;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            min-height: %6px;
            padding: %2px %5px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus {
            border-color: rgba(10, 132, 255, 170);
            background: rgba(255, 255, 255, 238);
        }
        QCheckBox {
            spacing: %2px;
            color: #1f2937;
        }
        QLabel {
            color: #263446;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background: transparent;
            border: 0;
            margin: %1px;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: rgba(92, 112, 138, 92);
            border-radius: %4px;
            min-height: %6px;
            min-width: %6px;
        }
        QScrollBar::handle:hover {
            background: rgba(70, 92, 120, 132);
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0;
            height: 0;
        }
    )")
        .arg(border)
        .arg(padV)
        .arg(pxValue(8, uiScale))
        .arg(smallRadius)
        .arg(padH)
        .arg(itemHeight)
        .arg(pxValue(14, uiScale))
        .arg(pxValue(7, uiScale))
        .arg(pxValue(5, uiScale))
        .arg(titleHeight)
        .arg(radius)
        .arg(pxValue(28, uiScale));
    if (isDarkTheme()) {
        sheet.replace("#fbfdff", "#111827");
        sheet.replace("#f2f7ff", "#101827");
        sheet.replace("#e8f1ff", "#0b1220");
        sheet.replace("#1f2937", "#e5edf7");
        sheet.replace("#263446", "#dbe7f5");
        sheet.replace("#233044", "#dbe7f5");
        sheet.replace("#526174", "#aab7c8");
        sheet.replace("#0b3d72", "#cde5ff");
        sheet.replace("rgba(255, 255, 255, 212)", "rgba(20, 28, 42, 226)");
        sheet.replace("rgba(255, 255, 255, 238)", "rgba(22, 31, 46, 245)");
        sheet.replace("rgba(255, 255, 255, 178)", "rgba(18, 27, 42, 220)");
        sheet.replace("rgba(255, 255, 255, 196)", "rgba(24, 34, 50, 232)");
        sheet.replace("rgba(255, 255, 255, 174)", "rgba(18, 27, 42, 224)");
        sheet.replace("rgba(255, 255, 255, 188)", "rgba(19, 28, 43, 224)");
        sheet.replace("rgba(255, 255, 255, 214)", "rgba(25, 36, 54, 238)");
        sheet.replace("rgba(255, 255, 255, 220)", "rgba(25, 36, 54, 235)");
        sheet.replace("rgba(255, 255, 255, 180)", "rgba(76, 96, 128, 100)");
        sheet.replace("rgba(255, 255, 255, 168)", "rgba(76, 96, 128, 88)");
        sheet.replace("rgba(255,255,255,238)", "rgba(35,45,63,238)");
        sheet.replace("rgba(255,255,255,250)", "rgba(44,57,78,245)");
        sheet.replace("rgba(228,238,252,220)", "rgba(24,34,50,232)");
        sheet.replace("rgba(218,235,255,232)", "rgba(36,54,78,238)");
        sheet.replace("rgba(232, 241, 255, 210)", "rgba(16, 24, 37, 224)");
        sheet.replace("rgba(190, 217, 249, 232)", "rgba(42, 61, 88, 235)");
    }
    return sheet;
}

ThemePreference themePreference()
{
    return g_themePreference;
}

QString themePreferenceName()
{
    switch (g_themePreference) {
    case ThemePreference::Light:
        return "light";
    case ThemePreference::Dark:
        return "dark";
    case ThemePreference::System:
    default:
        return "system";
    }
}

void setThemePreference(const QString& preference)
{
    const QString normalized = preference.trimmed().toLower();
    if (normalized == "light") {
        setThemePreference(ThemePreference::Light);
    } else if (normalized == "dark") {
        setThemePreference(ThemePreference::Dark);
    } else {
        setThemePreference(ThemePreference::System);
    }
}

void setThemePreference(ThemePreference preference)
{
    g_themePreference = preference;
}

bool isDarkTheme()
{
    if (g_themePreference == ThemePreference::Dark) {
        return true;
    }
    if (g_themePreference == ThemePreference::Light) {
        return false;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (auto* hints = QGuiApplication::styleHints()) {
        return hints->colorScheme() == Qt::ColorScheme::Dark;
    }
#endif
    return false;
}

void apply(QApplication& app, double uiScale)
{
    app.setFont(appFont(uiScale));
    app.setStyleSheet(styleSheet(uiScale));

    QPalette palette = app.palette();
    if (isDarkTheme()) {
        palette.setColor(QPalette::Window, QColor("#111827"));
        palette.setColor(QPalette::Base, QColor("#172033"));
        palette.setColor(QPalette::AlternateBase, QColor("#101827"));
        palette.setColor(QPalette::Text, QColor("#e5edf7"));
        palette.setColor(QPalette::ButtonText, QColor("#e5edf7"));
        palette.setColor(QPalette::Highlight, QColor("#60a5fa"));
    } else {
        palette.setColor(QPalette::Window, QColor("#f3f7ff"));
        palette.setColor(QPalette::Base, QColor("#ffffff"));
        palette.setColor(QPalette::AlternateBase, QColor("#edf4ff"));
        palette.setColor(QPalette::Text, QColor("#1f2937"));
        palette.setColor(QPalette::ButtonText, QColor("#1f2937"));
        palette.setColor(QPalette::Highlight, QColor("#0a84ff"));
    }
    app.setPalette(palette);
}

QGraphicsDropShadowEffect* makeShadow(QWidget* parent, double uiScale, int alpha)
{
    auto* effect = new QGraphicsDropShadowEffect(parent);
    effect->setBlurRadius(px(24, uiScale));
    effect->setOffset(0, px(6, uiScale));
    effect->setColor(QColor(54, 78, 116, alpha));
    return effect;
}

}
