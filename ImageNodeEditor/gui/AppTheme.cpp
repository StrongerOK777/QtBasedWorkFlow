#include "gui/AppTheme.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QWidget>

#include <algorithm>
#include <cmath>

#if defined(IMAGENODEEDITOR_USE_ELA)
#include "ElaTheme.h"
#endif

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
    m.toolbarIcon = px(24, uiScale);
    m.toolbarButton = px(30, uiScale);
    m.canvasZoomButtonW = px(46, uiScale);
    m.canvasZoomButtonH = px(40, uiScale);
    m.canvasZoomOverlayW = px(64, uiScale);
    m.canvasZoomOverlayH = px(100, uiScale);
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
    m.width = 250.0 * s;
    m.headerHeight = 42.0 * s;
    m.topPadding = 56.0 * s;
    m.rowHeight = 30.0 * s;
    m.bottomPadding = 22.0 * s;
    m.portRadius = 7.5 * s;
    m.cornerRadius = 18.0 * s;
    m.titleSize = 15.0 * s;
    m.labelSize = 13.0 * s;
    return m;
}

Colors colors()
{
    if (!isDarkTheme()) {
        return {};
    }
    Colors c;
    c.canvasTop = QColor("#191A1B");
    c.canvasBottom = QColor("#121314");
    c.canvasDot = QColor(220, 220, 220, 28);
    c.nodeTop = QColor(46, 46, 47, 148);
    c.nodeBottom = QColor(18, 19, 20, 132);
    c.nodeBorder = QColor(214, 214, 214, 96);
    c.nodeSelected = QColor("#f2f2f2");
    c.nodeShadow = QColor(0, 0, 0, 90);
    c.textPrimary = QColor("#e5edf7");
    c.textSecondary = QColor("#b8b8b8");
    c.inputPort = QColor("#30d158");
    c.outputPort = QColor("#d2d2d2");
    c.edge = QColor("#a8a8a8");
    c.edgeSelected = QColor("#ffb340");
    c.pendingEdge = QColor("#eeeeee");
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
    const int toolbarPad = px(1, uiScale);
    const int padH = px(10, uiScale);
    const int itemHeight = px(28, uiScale);
    const int titleHeight = px(34, uiScale);
    const int border = std::max(1, px(1, uiScale));

    QString sheet = QString(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #fbfdff, stop:0.55 #f2f7ff, stop:1 #e8f1ff);
        }
        QMainWindow::separator, QSplitter::handle {
            background: rgba(126, 154, 192, 74);
        }
        QDialog, QFrame, QWidget#canvasContainer {
            background: rgba(243, 247, 255, 232);
            color: #1f2937;
        }
        QAbstractScrollArea, QGraphicsView, QGraphicsView > QWidget {
            background: #edf4ff;
            border: 0;
            color: #1f2937;
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
            spacing: 0px;
            padding: 0px %13px;
            movable: false;
        }
        QToolBar QToolButton {
            min-width: %12px;
            min-height: %12px;
            padding: 0px;
            border: 0px solid transparent;
            border-radius: %1px;
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
        QTabBar#workbookTabs {
            background: transparent;
            qproperty-drawBase: 0;
        }
        QTabBar#workbookTabs::tab {
            min-height: %12px;
            padding: 0px %5px;
            margin: 0px %1px;
            border: %1px solid rgba(126, 154, 192, 86);
            border-radius: %4px;
            background: rgba(255, 255, 255, 128);
            color: #1f2937;
        }
        QTabBar#workbookTabs::tab:selected {
            background: rgba(10, 132, 255, 42);
            border-color: rgba(10, 132, 255, 118);
            color: #0b3d72;
        }
        QTabBar#workbookTabs::tab:hover {
            background: rgba(10, 132, 255, 28);
        }
        QTabWidget::pane {
            border: %1px solid rgba(126, 154, 192, 86);
            border-radius: %4px;
            background: rgba(255, 255, 255, 128);
        }
        QTabBar::tab {
            min-height: %12px;
            padding: 0px %5px;
            margin-right: %1px;
            border-top-left-radius: %4px;
            border-top-right-radius: %4px;
            background: rgba(255, 255, 255, 128);
            color: #1f2937;
        }
        QTabBar::tab:selected {
            background: rgba(10, 132, 255, 42);
            color: #0b3d72;
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
            background: rgba(243, 247, 255, 232);
            titlebar-close-icon: none;
            titlebar-normal-icon: none;
        }
        QDockWidget::separator {
            background: rgba(126, 154, 192, 74);
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
            color: #1f2937;
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
        QListWidget#logPanel, QListWidget#problemPanel, QTextEdit#logPanel, QPlainTextEdit#terminalOutput {
            background: rgba(255, 255, 255, 188);
            border-radius: %11px;
        }
        QListWidget, QTextEdit, QPlainTextEdit, QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            border: %1px solid rgba(126, 154, 192, 94);
            border-radius: %4px;
            background: rgba(255, 255, 255, 214);
            selection-background-color: rgba(10, 132, 255, 56);
            selection-color: #0b3d72;
            color: #1f2937;
        }
        QListWidget {
            padding: %1px;
            outline: 0;
        }
        QListWidget::item {
            min-height: %6px;
            border-radius: %4px;
            padding: %1px %5px;
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
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus, QPlainTextEdit:focus {
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
            background: rgba(126, 154, 192, 34);
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
        .arg(pxValue(30, uiScale))
        .arg(toolbarPad);
    if (isDarkTheme()) {
        sheet.replace("#fbfdff", "#2E2E2F");
        sheet.replace("#f2f7ff", "#191A1B");
        sheet.replace("#e8f1ff", "#121314");
        sheet.replace("#edf4ff", "#121314");
        sheet.replace("#1f2937", "#eeeeee");
        sheet.replace("#263446", "#dedede");
        sheet.replace("#233044", "#dedede");
        sheet.replace("#526174", "#b8b8b8");
        sheet.replace("#0b3d72", "#f2f2f2");
        sheet.replace("rgba(255, 255, 255, 212)", "rgba(46, 46, 47, 226)");
        sheet.replace("rgba(255, 255, 255, 238)", "rgba(46, 46, 47, 245)");
        sheet.replace("rgba(255, 255, 255, 178)", "rgba(25, 26, 27, 226)");
        sheet.replace("rgba(255, 255, 255, 196)", "rgba(46, 46, 47, 232)");
        sheet.replace("rgba(255, 255, 255, 174)", "rgba(25, 26, 27, 224)");
        sheet.replace("rgba(255, 255, 255, 188)", "rgba(25, 26, 27, 224)");
        sheet.replace("rgba(243, 247, 255, 232)", "rgba(18, 19, 20, 245)");
        sheet.replace("rgba(255, 255, 255, 214)", "rgba(46, 46, 47, 238)");
        sheet.replace("rgba(255, 255, 255, 220)", "rgba(46, 46, 47, 235)");
        sheet.replace("rgba(255, 255, 255, 180)", "rgba(160, 160, 160, 86)");
        sheet.replace("rgba(255, 255, 255, 168)", "rgba(160, 160, 160, 74)");
        sheet.replace("rgba(255, 255, 255, 128)", "rgba(46, 46, 47, 190)");
        sheet.replace("rgba(255,255,255,238)", "rgba(46,46,47,238)");
        sheet.replace("rgba(255,255,255,250)", "rgba(58,58,59,245)");
        sheet.replace("rgba(228,238,252,220)", "rgba(25,26,27,232)");
        sheet.replace("rgba(218,235,255,232)", "rgba(58,58,59,238)");
        sheet.replace("rgba(232, 241, 255, 210)", "rgba(25, 26, 27, 224)");
        sheet.replace("rgba(190, 217, 249, 232)", "rgba(70, 70, 71, 235)");
        sheet.replace("rgba(126, 154, 192, 34)", "rgba(85, 85, 86, 160)");
        sheet.replace("rgba(126, 154, 192, 74)", "rgba(110, 110, 111, 118)");
        sheet.replace("rgba(10, 132, 255, 28)", "rgba(238, 238, 238, 28)");
        sheet.replace("rgba(10, 132, 255, 34)", "rgba(238, 238, 238, 34)");
        sheet.replace("rgba(10, 132, 255, 42)", "rgba(238, 238, 238, 42)");
        sheet.replace("rgba(10, 132, 255, 52)", "rgba(238, 238, 238, 52)");
        sheet.replace("rgba(10, 132, 255, 56)", "rgba(238, 238, 238, 56)");
        sheet.replace("rgba(10, 132, 255, 58)", "rgba(238, 238, 238, 58)");
        sheet.replace("rgba(10, 132, 255, 96)", "rgba(238, 238, 238, 96)");
        sheet.replace("rgba(10, 132, 255, 118)", "rgba(238, 238, 238, 118)");
        sheet.replace("rgba(10, 132, 255, 148)", "rgba(238, 238, 238, 148)");
        sheet.replace("rgba(10, 132, 255, 170)", "rgba(238, 238, 238, 170)");
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
#if defined(IMAGENODEEDITOR_USE_ELA)
    eTheme->setThemeMode(isDarkTheme() ? ElaThemeType::Dark : ElaThemeType::Light);
#endif
    app.setStyleSheet(styleSheet(uiScale));

    QPalette palette = app.palette();
    if (isDarkTheme()) {
        palette.setColor(QPalette::Window, QColor("#121314"));
        palette.setColor(QPalette::WindowText, QColor("#eeeeee"));
        palette.setColor(QPalette::Button, QColor("#2E2E2F"));
        palette.setColor(QPalette::Base, QColor("#191A1B"));
        palette.setColor(QPalette::AlternateBase, QColor("#2E2E2F"));
        palette.setColor(QPalette::Text, QColor("#eeeeee"));
        palette.setColor(QPalette::ButtonText, QColor("#eeeeee"));
        palette.setColor(QPalette::BrightText, QColor("#ffffff"));
        palette.setColor(QPalette::Highlight, QColor("#d6d6d6"));
        palette.setColor(QPalette::HighlightedText, QColor("#121314"));
        palette.setColor(QPalette::ToolTipBase, QColor("#2E2E2F"));
        palette.setColor(QPalette::ToolTipText, QColor("#eeeeee"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setColor(QPalette::PlaceholderText, QColor("#a8a8a8"));
#endif
    } else {
        palette.setColor(QPalette::Window, QColor("#f3f7ff"));
        palette.setColor(QPalette::WindowText, QColor("#1f2937"));
        palette.setColor(QPalette::Button, QColor("#edf4ff"));
        palette.setColor(QPalette::Base, QColor("#ffffff"));
        palette.setColor(QPalette::AlternateBase, QColor("#edf4ff"));
        palette.setColor(QPalette::Text, QColor("#1f2937"));
        palette.setColor(QPalette::ButtonText, QColor("#1f2937"));
        palette.setColor(QPalette::BrightText, QColor("#0b3d72"));
        palette.setColor(QPalette::Highlight, QColor("#0a84ff"));
        palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipText, QColor("#1f2937"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setColor(QPalette::PlaceholderText, QColor("#6b7b8f"));
#endif
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
