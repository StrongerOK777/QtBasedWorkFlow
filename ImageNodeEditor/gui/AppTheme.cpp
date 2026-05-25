#include "gui/AppTheme.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPalette>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace {

AppTheme::ThemePreference g_themePreference = AppTheme::ThemePreference::Dark;

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
    m.cornerRadius = 0.0;
    m.titleSize = 15.0 * s;
    m.labelSize = 13.0 * s;
    return m;
}

Colors colors()
{
    Colors c;
    c.canvasTop = QColor("#1e1e1e");
    c.canvasBottom = QColor("#1e1e1e");
    c.canvasDot = QColor(204, 204, 204, 24);
    c.nodeTop = QColor("#252526");
    c.nodeBottom = QColor("#1f1f1f");
    c.nodeBorder = QColor("#3c3c3c");
    c.nodeSelected = QColor("#3794ff");
    c.nodeShadow = QColor(0, 0, 0, 120);
    c.textPrimary = QColor("#cccccc");
    c.textSecondary = QColor("#969696");
    c.inputPort = QColor("#9cdcfe");
    c.outputPort = QColor("#dcdcaa");
    c.edge = QColor("#6a9955");
    c.edgeSelected = QColor("#3794ff");
    c.pendingEdge = QColor("#3794ff");
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
    const int radius = 0;
    const int smallRadius = 0;
    const int padV = px(6, uiScale);
    const int toolbarPad = px(1, uiScale);
    const int padH = px(10, uiScale);
    const int itemHeight = px(28, uiScale);
    const int titleHeight = px(34, uiScale);
    const int border = std::max(1, px(1, uiScale));

    return QString(R"(
        QMainWindow, QWidget#workbenchHost, QWidget#editorShell {
            background: #1e1e1e;
            color: #cccccc;
        }
        QMainWindow::separator, QSplitter::handle {
            background: #2d2d2d;
        }
        QDialog, QFrame, QWidget#canvasContainer, QWidget#bottomPanel, QWidget#bottomTabPage {
            background: #1e1e1e;
            color: #cccccc;
            border: 0px;
        }
        QWidget#previewSidebar {
            background: #252526;
            border-left: %1px solid #2d2d2d;
            color: #cccccc;
        }
        QLabel#workbenchPanelTitle {
            color: #cccccc;
            font-weight: 600;
        }
        QAbstractScrollArea, QGraphicsView, QGraphicsView > QWidget {
            background: #1e1e1e;
            color: #cccccc;
            border: 0px;
        }
        QMenuBar, QMenu {
            background: #252526;
            color: #cccccc;
            border: %1px solid #454545;
        }
        QMenu::item {
            min-height: %2px;
            padding: %3px %4px;
            background: transparent;
        }
        QMenu::item:selected {
            background: #04395e;
            color: #ffffff;
        }
        QToolBar {
            background: #252526;
            border: 0px;
            border-bottom: %1px solid #2d2d2d;
            spacing: 0px;
            padding: 0px;
        }
        QToolButton, QPushButton {
            min-height: %2px;
            padding: %3px %4px;
            border: %1px solid #454545;
            border-radius: 0px;
            background: #2d2d2d;
            color: #cccccc;
        }
        QToolButton:hover, QPushButton:hover {
            background: #3c3c3c;
            border-color: #555555;
        }
        QToolButton:pressed, QPushButton:pressed {
            background: #094771;
            border-color: #3794ff;
        }
        QTabWidget::pane {
            border-top: %1px solid #2d2d2d;
            background: #1e1e1e;
        }
        QTabBar::tab {
            min-height: %5px;
            padding: 0px %4px;
            margin: 0px;
            border: 0px;
            border-right: %1px solid #2d2d2d;
            background: #2d2d2d;
            color: #969696;
        }
        QTabBar::tab:selected {
            background: #1e1e1e;
            color: #ffffff;
        }
        QLabel#previewPanel {
            background: #1e1e1e;
            border: %1px solid #454545;
            color: #969696;
        }
        QListWidget#logPanel, QListWidget#problemPanel, QPlainTextEdit#terminalOutput {
            background: #1e1e1e;
            border: 0px;
            color: #cccccc;
        }
        QListWidget, QTextEdit, QPlainTextEdit, QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            border: %1px solid #454545;
            border-radius: 0px;
            background: #3c3c3c;
            selection-background-color: #04395e;
            selection-color: #ffffff;
            color: #cccccc;
        }
        QListWidget::item {
            min-height: %2px;
            padding: %1px %4px;
        }
        QListWidget::item:hover {
            background: #2a2d2e;
        }
        QListWidget::item:selected {
            background: #04395e;
            color: #ffffff;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            min-height: %2px;
            padding: %3px %4px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border-color: #3794ff;
            background: #3c3c3c;
        }
        QCheckBox, QLabel {
            color: #cccccc;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background: #1e1e1e;
            border: 0;
            margin: 0px;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: #424242;
            border-radius: 0px;
            min-height: %2px;
            min-width: %2px;
        }
        QScrollBar::handle:hover {
            background: #5a5a5a;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0px;
            height: 0px;
        }
        QWidget#canvasZoomOverlay {
            background: rgba(37, 37, 38, 210);
            border: %1px solid #454545;
        }
        QToolTip {
            background: #252526;
            color: #cccccc;
            border: %1px solid #454545;
            padding: %3px %4px;
        }
    )")
        .arg(border)
        .arg(itemHeight)
        .arg(padV)
        .arg(padH)
        .arg(px(32, uiScale));

    QString sheet = QString(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ffffff, stop:0.55 #f5f5f5, stop:1 #ececec);
        }
        QMainWindow::separator, QSplitter::handle {
            background: rgba(128, 128, 128, 74);
        }
        QDialog, QFrame, QWidget#canvasContainer {
            background: rgba(245, 245, 245, 232);
            color: #202020;
        }
        QWidget#workbench, QWidget#sidebarStack, QWidget#nodeLibraryPage, QWidget#workflowPage {
            background: rgba(245, 245, 245, 232);
            color: #202020;
        }
        QWidget#activityBar {
            background: rgba(236, 236, 236, 242);
            border-right: %1px solid rgba(128, 128, 128, 86);
        }
        QWidget#primarySidebar, QWidget#previewSidebar, QWidget#bottomPanel, QWidget#bottomTabPage {
            background: rgba(255, 255, 255, 174);
            border: 0px;
            color: #202020;
        }
        QWidget#primarySidebar {
            border-right: %1px solid rgba(128, 128, 128, 86);
        }
        QWidget#previewSidebar {
            border-left: %1px solid rgba(128, 128, 128, 86);
        }
        QWidget#bottomPanel {
            border-top: %1px solid rgba(128, 128, 128, 86);
        }
        QLabel#workbenchPanelTitle {
            font-weight: 600;
            color: #202020;
        }
        QToolButton#activityButton {
            min-width: %12px;
            min-height: %12px;
            padding: 0px;
            border: 0px;
            border-left: %1px solid transparent;
            background: transparent;
        }
        QToolButton#activityButton:hover {
            background: rgba(64, 64, 64, 28);
        }
        QToolButton#activityButton:checked {
            border-left-color: rgba(32, 32, 32, 196);
            background: rgba(64, 64, 64, 42);
        }
        QAbstractScrollArea, QGraphicsView, QGraphicsView > QWidget {
            background: #f2f2f2;
            border: 0;
            color: #202020;
        }
        QMenuBar {
            background: rgba(255, 255, 255, 212);
            border-bottom: %1px solid rgba(128, 128, 128, 90);
            padding: %2px %3px;
            color: #202020;
        }
        QMenuBar::item {
            border-radius: %4px;
            padding: %2px %5px;
            background: transparent;
        }
        QMenuBar::item:selected {
            background: rgba(64, 64, 64, 34);
        }
        QMenu {
            background: rgba(255, 255, 255, 238);
            border: %1px solid rgba(128, 128, 128, 90);
            border-radius: %11px;
            padding: %2px;
            color: #202020;
        }
        QMenu::item {
            min-height: %6px;
            border-radius: 0px;
            padding: %2px %7px;
        }
        QMenu::item:selected {
            background: rgba(64, 64, 64, 42);
            color: #111111;
        }
        QToolBar {
            background: rgba(255, 255, 255, 178);
            border: %1px solid rgba(255, 255, 255, 180);
            border-bottom-color: rgba(128, 128, 128, 82);
            spacing: 0px;
            padding: 0px %13px;
        }
        QToolBar#headerToolbar QWidget#windowHeader {
            background: transparent;
        }
        QLabel#documentTitleLabel {
            font-weight: 600;
            color: #202020;
        }
        QToolBar QToolButton {
            min-width: %12px;
            min-height: %12px;
            padding: 0px;
            border: 0px solid transparent;
            border-radius: 0px;
            background: transparent;
        }
        QToolButton, QPushButton {
            min-height: %6px;
            border-radius: 0px;
            padding: %2px %5px;
            border: %1px solid rgba(128, 128, 128, 88);
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,238), stop:1 rgba(232,232,232,220));
            color: #202020;
        }
        QToolBar QToolButton:hover {
            border-color: rgba(64, 64, 64, 96);
            background: rgba(64, 64, 64, 28);
        }
        QToolBar QToolButton:pressed {
            background: rgba(64, 64, 64, 52);
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
            border: %1px solid rgba(128, 128, 128, 86);
            border-radius: 0px;
            background: rgba(255, 255, 255, 128);
            color: #202020;
        }
        QTabBar#workbookTabs::tab:selected {
            background: rgba(64, 64, 64, 42);
            border-color: rgba(64, 64, 64, 118);
            color: #111111;
        }
        QTabBar#workbookTabs::tab:hover {
            background: rgba(64, 64, 64, 28);
        }
        QTabWidget::pane {
            border: %1px solid rgba(128, 128, 128, 86);
            border-radius: 0px;
            background: rgba(255, 255, 255, 128);
        }
        QTabBar::tab {
            min-height: %12px;
            padding: 0px %5px;
            margin-right: %1px;
            border-top-left-radius: 0px;
            border-top-right-radius: 0px;
            background: rgba(255, 255, 255, 128);
            color: #202020;
        }
        QTabBar::tab:selected {
            background: rgba(64, 64, 64, 42);
            color: #111111;
        }
        QToolButton:hover, QPushButton:hover {
            border-color: rgba(64, 64, 64, 148);
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,250), stop:1 rgba(220,220,220,232));
        }
        QToolButton:pressed, QPushButton:pressed {
            background: rgba(210, 210, 210, 232);
            padding-top: %8px;
            padding-bottom: %9px;
        }
        QDockWidget {
            color: #202020;
            background: rgba(245, 245, 245, 232);
            titlebar-close-icon: none;
            titlebar-normal-icon: none;
        }
        QDockWidget::separator {
            background: rgba(128, 128, 128, 74);
        }
        QDockWidget::title {
            min-height: %10px;
            padding-left: %7px;
            text-align: left;
            background: rgba(255, 255, 255, 196);
            border: %1px solid rgba(255, 255, 255, 188);
            border-bottom-color: rgba(128, 128, 128, 78);
            border-top-left-radius: 0px;
            border-top-right-radius: 0px;
        }
        QDockWidget > QWidget, QWidget#glassPanel {
            background: rgba(255, 255, 255, 174);
            border: %1px solid rgba(255, 255, 255, 168);
            border-radius: 0px;
            color: #202020;
        }
        QWidget#canvasZoomOverlay {
            background: rgba(255, 255, 255, 178);
            border: %1px solid rgba(128, 128, 128, 86);
            border-radius: 0px;
        }
        QLabel#previewPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255, 255, 255, 220), stop:1 rgba(232, 232, 232, 210));
            border: %1px solid rgba(128, 128, 128, 88);
            border-radius: 0px;
            color: #606060;
        }
        QListWidget#logPanel, QListWidget#problemPanel, QTextEdit#logPanel, QPlainTextEdit#terminalOutput {
            background: rgba(255, 255, 255, 188);
            border-radius: 0px;
        }
        QListWidget, QTextEdit, QPlainTextEdit, QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            border: %1px solid rgba(128, 128, 128, 94);
            border-radius: 0px;
            background: rgba(255, 255, 255, 214);
            selection-background-color: rgba(64, 64, 64, 56);
            selection-color: #111111;
            color: #202020;
        }
        QListWidget {
            padding: %1px;
            outline: 0;
        }
        QListWidget::item {
            min-height: %6px;
            border-radius: 0px;
            padding: %1px %5px;
        }
        QListWidget::item:hover {
            background: rgba(64, 64, 64, 28);
        }
        QListWidget::item:selected {
            background: rgba(64, 64, 64, 58);
            color: #111111;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            min-height: %6px;
            padding: %2px %5px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border-color: rgba(64, 64, 64, 170);
            background: rgba(255, 255, 255, 238);
        }
        QCheckBox {
            spacing: %2px;
            color: #202020;
        }
        QLabel {
            color: #303030;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background: rgba(128, 128, 128, 34);
            border: 0;
            margin: %1px;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: rgba(96, 96, 96, 92);
            border-radius: 0px;
            min-height: %6px;
            min-width: %6px;
        }
        QScrollBar::handle:hover {
            background: rgba(72, 72, 72, 132);
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
        sheet.replace("#ffffff", "#2E2E2F");
        sheet.replace("#f5f5f5", "#191A1B");
        sheet.replace("#ececec", "#121314");
        sheet.replace("#f2f2f2", "#121314");
        sheet.replace("#202020", "#eeeeee");
        sheet.replace("#303030", "#dedede");
        sheet.replace("#606060", "#b8b8b8");
        sheet.replace("#111111", "#f2f2f2");
        sheet.replace("rgba(255, 255, 255, 212)", "rgba(46, 46, 47, 226)");
        sheet.replace("rgba(255, 255, 255, 238)", "rgba(46, 46, 47, 245)");
        sheet.replace("rgba(255, 255, 255, 178)", "rgba(25, 26, 27, 226)");
        sheet.replace("rgba(255, 255, 255, 196)", "rgba(46, 46, 47, 232)");
        sheet.replace("rgba(255, 255, 255, 174)", "rgba(25, 26, 27, 224)");
        sheet.replace("rgba(255, 255, 255, 188)", "rgba(25, 26, 27, 224)");
        sheet.replace("rgba(245, 245, 245, 232)", "rgba(18, 19, 20, 245)");
        sheet.replace("rgba(236, 236, 236, 242)", "rgba(18, 19, 20, 250)");
        sheet.replace("rgba(255, 255, 255, 214)", "rgba(46, 46, 47, 238)");
        sheet.replace("rgba(255, 255, 255, 220)", "rgba(46, 46, 47, 235)");
        sheet.replace("rgba(255, 255, 255, 180)", "rgba(160, 160, 160, 86)");
        sheet.replace("rgba(255, 255, 255, 168)", "rgba(160, 160, 160, 74)");
        sheet.replace("rgba(255, 255, 255, 128)", "rgba(46, 46, 47, 190)");
        sheet.replace("rgba(255,255,255,238)", "rgba(46,46,47,238)");
        sheet.replace("rgba(255,255,255,250)", "rgba(58,58,59,245)");
        sheet.replace("rgba(232,232,232,220)", "rgba(25,26,27,232)");
        sheet.replace("rgba(220,220,220,232)", "rgba(58,58,59,238)");
        sheet.replace("rgba(232, 232, 232, 210)", "rgba(25, 26, 27, 224)");
        sheet.replace("rgba(210, 210, 210, 232)", "rgba(70, 70, 71, 235)");
        sheet.replace("rgba(128, 128, 128, 34)", "rgba(85, 85, 86, 160)");
        sheet.replace("rgba(128, 128, 128, 74)", "rgba(110, 110, 111, 118)");
        sheet.replace("rgba(64, 64, 64, 28)", "rgba(238, 238, 238, 28)");
        sheet.replace("rgba(64, 64, 64, 34)", "rgba(238, 238, 238, 34)");
        sheet.replace("rgba(64, 64, 64, 42)", "rgba(238, 238, 238, 42)");
        sheet.replace("rgba(64, 64, 64, 52)", "rgba(238, 238, 238, 52)");
        sheet.replace("rgba(64, 64, 64, 56)", "rgba(238, 238, 238, 56)");
        sheet.replace("rgba(64, 64, 64, 58)", "rgba(238, 238, 238, 58)");
        sheet.replace("rgba(64, 64, 64, 96)", "rgba(238, 238, 238, 96)");
        sheet.replace("rgba(64, 64, 64, 118)", "rgba(238, 238, 238, 118)");
        sheet.replace("rgba(64, 64, 64, 148)", "rgba(238, 238, 238, 148)");
        sheet.replace("rgba(64, 64, 64, 170)", "rgba(238, 238, 238, 170)");
        sheet.replace("rgba(32, 32, 32, 196)", "rgba(238, 238, 238, 196)");
    }
    return sheet;
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
    palette.setColor(QPalette::Window, QColor("#1e1e1e"));
    palette.setColor(QPalette::WindowText, QColor("#cccccc"));
    palette.setColor(QPalette::Button, QColor("#2d2d2d"));
    palette.setColor(QPalette::Base, QColor("#1e1e1e"));
    palette.setColor(QPalette::AlternateBase, QColor("#252526"));
    palette.setColor(QPalette::Text, QColor("#cccccc"));
    palette.setColor(QPalette::ButtonText, QColor("#cccccc"));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));
    palette.setColor(QPalette::Highlight, QColor("#04395e"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipBase, QColor("#252526"));
    palette.setColor(QPalette::ToolTipText, QColor("#cccccc"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    palette.setColor(QPalette::PlaceholderText, QColor("#969696"));
#endif
    app.setPalette(palette);
}

QGraphicsDropShadowEffect* makeShadow(QWidget* parent, double uiScale, int alpha)
{
    auto* effect = new QGraphicsDropShadowEffect(parent);
    effect->setBlurRadius(px(24, uiScale));
    effect->setOffset(0, px(6, uiScale));
    effect->setColor(QColor(0, 0, 0, alpha));
    return effect;
}

}
