#include "gui/AppTheme.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPalette>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace {

AppTheme::ThemePreference g_themePreference = AppTheme::ThemePreference::Dark;

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
    m.cornerRadius = 12.0 * s;
    m.titleSize = 15.0 * s;
    m.labelSize = 13.0 * s;
    return m;
}

Colors colors()
{
    // 冷静中性深底 + 柔化蓝点缀（与全局 QSS token 保持一致）。
    Colors c;
    c.canvasTop = QColor("#1b1c1e");
    c.canvasBottom = QColor("#1b1c1e");
    c.canvasDot = QColor(255, 255, 255, 16);
    c.nodeTop = QColor("#26282d");
    c.nodeBottom = QColor("#1f2024");
    c.nodeBorder = QColor("#34363b");
    c.nodeSelected = QColor("#6ea0e0");
    c.nodeShadow = QColor(0, 0, 0, 110);
    c.textPrimary = QColor("#e3e4e6");
    c.textSecondary = QColor("#9aa0a6");
    c.inputPort = QColor("#9cc6f0");
    c.outputPort = QColor("#d9c98f");
    c.edge = QColor("#6e7a86");
    c.edgeSelected = QColor("#6ea0e0");
    c.pendingEdge = QColor("#6ea0e0");
    return c;
}

QFont appFont(double uiScale)
{
    QFont font = QApplication::font();
    font.setPointSizeF(std::max(9.0, 12.0 * clampedScale(uiScale)));
    return font;
}

QString styleSheet(double uiScale)
{
    const int padV = px(6, uiScale);
    const int padH = px(11, uiScale);
    const int itemHeight = px(28, uiScale);
    const int tabHeight = px(32, uiScale);
    const int border = std::max(1, px(1, uiScale));

    // 设计 token：冷静中性深底（base #1b1c1e / panel #212327 / elevated #26282d /
    // input #2c2f34），hairline 边 #2e2f33，柔化蓝点缀 #6ea0e0，选中染色 #303a47，
    // 圆角 sm6 / md8 / lg12。
    return QString(R"(
        QMainWindow, QWidget#workbenchHost, QWidget#editorShell {
            background: #1b1c1e;
            color: #e3e4e6;
        }
        QMainWindow::separator, QSplitter::handle {
            background: #2e2f33;
        }
        QDialog, QFrame, QWidget#canvasContainer, QWidget#bottomPanel, QWidget#bottomTabPage {
            background: #1b1c1e;
            color: #e3e4e6;
            border: 0px;
        }
        QWidget#previewSidebar {
            background: #212327;
            border-left: %1px solid #2e2f33;
            color: #e3e4e6;
        }
        QLabel#workbenchPanelTitle {
            color: #c8cace;
            font-weight: 600;
        }
        QAbstractScrollArea, QGraphicsView, QGraphicsView > QWidget {
            background: #1b1c1e;
            color: #e3e4e6;
            border: 0px;
        }
        QMenuBar {
            background: #1b1c1e;
            color: #e3e4e6;
            border: 0px;
        }
        QMenuBar::item {
            padding: %3px %4px;
            background: transparent;
            border-radius: 6px;
        }
        QMenuBar::item:selected {
            background: #26282d;
        }
        QMenu {
            background: #212327;
            color: #e3e4e6;
            border: %1px solid #2e2f33;
            border-radius: 8px;
            padding: 4px;
        }
        QMenu::item {
            min-height: %2px;
            padding: %3px %4px;
            background: transparent;
            border-radius: 6px;
        }
        QMenu::item:selected {
            background: #303a47;
            color: #ffffff;
        }
        QMenu::separator {
            height: 1px;
            background: #2e2f33;
            margin: 4px 8px;
        }
        QToolBar {
            background: #1b1c1e;
            border: 0px;
            border-bottom: %1px solid #2e2f33;
            spacing: 2px;
            padding: 2px;
        }
        QToolButton, QPushButton {
            min-height: %2px;
            padding: %3px %4px;
            border: %1px solid #2e2f33;
            border-radius: 8px;
            background: #26282d;
            color: #e3e4e6;
        }
        QToolButton:hover, QPushButton:hover {
            background: #303237;
            border-color: #3b3d42;
        }
        QToolButton:pressed, QPushButton:pressed {
            background: #303a47;
            border-color: #6ea0e0;
        }
        QToolButton:disabled, QPushButton:disabled {
            color: #6b7178;
            background: #212327;
        }
        QTabWidget::pane {
            border-top: %1px solid #2e2f33;
            background: #1b1c1e;
        }
        QTabBar::tab {
            min-height: %5px;
            padding: 0px %4px;
            margin: 2px 1px 0px 1px;
            border: 0px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            background: transparent;
            color: #9aa0a6;
        }
        QTabBar::tab:hover {
            background: #26282d;
        }
        QTabBar::tab:selected {
            background: #26282d;
            color: #ffffff;
        }
        QLabel#previewPanel {
            background: #1b1c1e;
            border: %1px solid #2e2f33;
            border-radius: 10px;
            color: #9aa0a6;
        }
        QListWidget#logPanel, QListWidget#problemPanel, QPlainTextEdit#terminalOutput {
            background: #1b1c1e;
            border: 0px;
            color: #d6d8db;
        }
        QListWidget, QTextEdit, QPlainTextEdit, QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            border: %1px solid #2e2f33;
            border-radius: 6px;
            background: #2c2f34;
            selection-background-color: #35506e;
            selection-color: #ffffff;
            color: #e3e4e6;
        }
        QListWidget::item {
            min-height: %2px;
            padding: %1px %4px;
            border-radius: 6px;
        }
        QListWidget::item:hover {
            background: #26282d;
        }
        QListWidget::item:selected {
            background: #303a47;
            color: #ffffff;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            min-height: %2px;
            padding: %3px %4px;
        }
        QComboBox::drop-down {
            border: 0px;
            width: 18px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border-color: #6ea0e0;
            background: #2c2f34;
        }
        QCheckBox, QLabel {
            color: #e3e4e6;
        }
        QScrollBar:vertical {
            background: transparent;
            border: 0;
            width: 11px;
            margin: 2px;
        }
        QScrollBar:horizontal {
            background: transparent;
            border: 0;
            height: 11px;
            margin: 2px;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: #3a3c40;
            border-radius: 4px;
            min-height: %2px;
            min-width: %2px;
        }
        QScrollBar::handle:hover {
            background: #4a4d52;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0px;
            height: 0px;
        }
        QScrollBar::add-page, QScrollBar::sub-page {
            background: transparent;
        }
        QWidget#canvasZoomOverlay {
            background: rgba(33, 35, 39, 235);
            border: %1px solid #2e2f33;
            border-radius: 12px;
        }
        QToolTip {
            background: #26282d;
            color: #e3e4e6;
            border: %1px solid #34363b;
            border-radius: 8px;
            padding: %3px %4px;
        }
        QWidget#canvasTabStrip {
            background: #1b1c1e;
            border-bottom: %1px solid #2e2f33;
        }
        QWidget#breadcrumbBar {
            background: #1b1c1e;
            border-bottom: %1px solid #2e2f33;
        }
        QTabBar#workbookTabs::tab {
            min-height: %5px;
            padding: 3px 12px;
            margin: 0px;
            border: 0px;
            border-top-left-radius: 7px;
            border-top-right-radius: 7px;
            background: transparent;
            color: #9aa0a6;
        }
        QTabBar#workbookTabs::tab:hover {
            background: #26282d;
        }
        QTabBar#workbookTabs::tab:selected {
            background: #2a2c31;
            color: #e8eaed;
        }
        QToolButton#canvasNewButton {
            border: 0px;
            border-radius: 6px;
            background: transparent;
            padding: 2px;
        }
        QToolButton#canvasNewButton:hover {
            background: #26282d;
        }
        QToolButton#breadcrumbNav {
            border: 0px;
            background: transparent;
            border-radius: 6px;
            padding: 2px 5px;
        }
        QToolButton#breadcrumbNav:hover {
            background: #26282d;
        }
        QToolButton#breadcrumbSegment {
            border: 0px;
            background: transparent;
            border-radius: 6px;
            padding: 1px 7px;
            color: #9aa0a6;
        }
        QToolButton#breadcrumbSegment:hover {
            background: #26282d;
            color: #c8cace;
        }
        QToolButton#breadcrumbCurrent {
            border: 0px;
            background: transparent;
            padding: 1px 7px;
            color: #e3e4e6;
            font-weight: 600;
        }
        QLabel#breadcrumbSep {
            color: #565b61;
            padding: 0px 1px;
        }
    )")
        .arg(border)
        .arg(itemHeight)
        .arg(padV)
        .arg(padH)
        .arg(tabHeight);
}

ThemePreference themePreference()
{
    return g_themePreference;
}

QString themePreferenceName()
{
    return "dark";
}

void setThemePreference(const QString& preference)
{
    Q_UNUSED(preference);
    setThemePreference(ThemePreference::Dark);
}

void setThemePreference(ThemePreference preference)
{
    Q_UNUSED(preference);
    g_themePreference = ThemePreference::Dark;
}

bool isDarkTheme()
{
    return true;
}

void apply(QApplication& app, double uiScale)
{
    app.setFont(appFont(uiScale));
    app.setStyleSheet(styleSheet(uiScale));

    QPalette palette = app.palette();
    palette.setColor(QPalette::Window, QColor("#1b1c1e"));
    palette.setColor(QPalette::WindowText, QColor("#e3e4e6"));
    palette.setColor(QPalette::Button, QColor("#26282d"));
    palette.setColor(QPalette::Base, QColor("#1b1c1e"));
    palette.setColor(QPalette::AlternateBase, QColor("#212327"));
    palette.setColor(QPalette::Text, QColor("#e3e4e6"));
    palette.setColor(QPalette::ButtonText, QColor("#e3e4e6"));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));
    palette.setColor(QPalette::Highlight, QColor("#35506e"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipBase, QColor("#26282d"));
    palette.setColor(QPalette::ToolTipText, QColor("#e3e4e6"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    palette.setColor(QPalette::PlaceholderText, QColor("#6b7178"));
#endif
    app.setPalette(palette);
}

QGraphicsDropShadowEffect* makeShadow(QWidget* parent, double uiScale, int alpha)
{
    auto* effect = new QGraphicsDropShadowEffect(parent);
    effect->setBlurRadius(px(28, uiScale));
    effect->setOffset(0, px(8, uiScale));
    effect->setColor(QColor(0, 0, 0, alpha));
    return effect;
}

}
