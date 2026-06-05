#include "gui/AppTheme.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPalette>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace {

AppTheme::ThemePreference g_themePreference = AppTheme::ThemePreference::Dark;

QString rgba(const QColor& c)
{
    return QString("rgba(%1, %2, %3, %4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

}

namespace AppTheme {

Palette darkPalette()
{
    // 冷静中性深底 + 柔化蓝点缀（沿用既有 VS Code Dark token，避免深色回归）。
    Palette p;
    p.base = QColor("#1b1c1e");
    p.panel = QColor("#212327");
    p.elevated = QColor("#26282d");
    p.elevatedHover = QColor("#303237");
    p.input = QColor("#2c2f34");
    p.hairline = QColor("#2e2f33");
    p.border = QColor("#3b3d42");
    p.textPrimary = QColor("#e3e4e6");
    p.textSecondary = QColor("#9aa0a6");
    p.textMuted = QColor("#6b7178");
    p.accent = QColor("#6ea0e0");
    p.accentHover = QColor("#7fb0ee");
    p.selection = QColor("#303a47");
    p.selectionText = QColor("#ffffff");
    p.onAccent = QColor("#ffffff");
    p.warning = QColor("#d9c98f");
    p.danger = QColor("#e0918c");
    p.scrollHandle = QColor("#3a3c40");
    p.scrollHandleHover = QColor("#4a4d52");
    p.canvasBg = QColor("#1b1c1e");
    p.canvasDot = QColor(255, 255, 255, 16);
    p.nodeTop = QColor("#26282d");
    p.nodeBottom = QColor("#1f2024");
    p.nodeHeader = QColor("#2a2c31");
    p.nodeBorder = QColor("#34363b");
    p.nodeSelected = QColor("#6ea0e0");
    p.nodeShadow = QColor(0, 0, 0, 110);
    p.nodeText = QColor("#e3e4e6");
    p.nodeTextFaded = QColor("#9aa0a6");
    p.inputPort = QColor("#9cc6f0");
    p.outputPort = QColor("#d9c98f");
    p.edge = QColor("#6e7a86");
    p.edgeSelected = QColor("#6ea0e0");
    p.pendingEdge = QColor("#6ea0e0");
    return p;
}

Palette lightPalette()
{
    // 浅色主题：背景 #F8F8F6 / 文字 #0A0A0A，绿色为主强调、暗黄为次强调。
    Palette p;
    p.base = QColor("#f8f8f6");
    p.panel = QColor("#efeee9");
    p.elevated = QColor("#e7e6e0");
    p.elevatedHover = QColor("#dfded7");
    p.input = QColor("#ffffff");
    p.hairline = QColor("#dedcd3");
    p.border = QColor("#cdcbc1");
    p.textPrimary = QColor("#0a0a0a");
    p.textSecondary = QColor("#55554f");
    p.textMuted = QColor("#8a897f");
    p.accent = QColor("#2e9e5b");
    p.accentHover = QColor("#38b069");
    p.selection = QColor("#ddefe2");
    p.selectionText = QColor("#0a0a0a");
    p.onAccent = QColor("#ffffff");
    p.warning = QColor("#b58a0c");
    p.danger = QColor("#c0392b");
    p.scrollHandle = QColor("#cbcac0");
    p.scrollHandleHover = QColor("#b5b4a9");
    p.canvasBg = QColor("#f8f8f6");
    p.canvasDot = QColor(10, 10, 10, 22);
    p.nodeTop = QColor("#ffffff");
    p.nodeBottom = QColor("#f2f1eb");
    p.nodeHeader = QColor("#efefe9");
    p.nodeBorder = QColor("#d5d3c9");
    p.nodeSelected = QColor("#2e9e5b");
    p.nodeShadow = QColor(0, 0, 0, 38);
    p.nodeText = QColor("#0a0a0a");
    p.nodeTextFaded = QColor("#6e6d65");
    p.inputPort = QColor("#3e8e5e");
    p.outputPort = QColor("#b58a0c");
    p.edge = QColor("#8c8b80");
    p.edgeSelected = QColor("#2e9e5b");
    p.pendingEdge = QColor("#2e9e5b");
    return p;
}

Palette palette()
{
    return g_themePreference == ThemePreference::Light ? lightPalette() : darkPalette();
}

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
    m.titleSize = 16.0 * s;
    m.labelSize = 14.0 * s;
    return m;
}

Colors colors()
{
    const Palette p = palette();
    Colors c;
    c.canvasTop = p.canvasBg;
    c.canvasBottom = p.canvasBg;
    c.canvasDot = p.canvasDot;
    c.nodeTop = p.nodeTop;
    c.nodeBottom = p.nodeBottom;
    c.nodeHeader = p.nodeHeader;
    c.nodeBorder = p.nodeBorder;
    c.nodeSelected = p.nodeSelected;
    c.nodeShadow = p.nodeShadow;
    c.textPrimary = p.nodeText;
    c.textSecondary = p.nodeTextFaded;
    c.inputPort = p.inputPort;
    c.outputPort = p.outputPort;
    c.edge = p.edge;
    c.edgeSelected = p.edgeSelected;
    c.pendingEdge = p.pendingEdge;
    return c;
}

QFont appFont(double uiScale)
{
    QFont font = QApplication::font();
    font.setPointSizeF(std::max(10.0, 13.0 * clampedScale(uiScale)));
    return font;
}

QString styleSheet(double uiScale)
{
    const int padV = px(6, uiScale);
    const int padH = px(11, uiScale);
    const int itemHeight = px(28, uiScale);
    const int tabHeight = px(32, uiScale);
    const int border = std::max(1, px(1, uiScale));

    // 设计 token 由活动 palette 提供，深色 / 浅色共用同一份 QSS 模板，只换 token 取值。
    QString sheet = QString(R"(
        QMainWindow, QWidget#workbenchHost, QWidget#editorShell {
            background: @base@;
            color: @textPrimary@;
        }
        QMainWindow::separator, QSplitter::handle {
            background: @hairline@;
        }
        QDialog, QFrame, QWidget#canvasContainer, QWidget#bottomPanel, QWidget#bottomTabPage {
            background: @base@;
            color: @textPrimary@;
            border: 0px;
        }
        QWidget#previewSidebar {
            background: @panel@;
            border-left: %1px solid @hairline@;
            color: @textPrimary@;
        }
        QLabel#workbenchPanelTitle {
            color: @textPrimary@;
            font-weight: 600;
        }
        QAbstractScrollArea, QGraphicsView, QGraphicsView > QWidget {
            background: @base@;
            color: @textPrimary@;
            border: 0px;
        }
        QMenuBar {
            background: @base@;
            color: @textPrimary@;
            border: 0px;
        }
        QMenuBar::item {
            padding: %3px %4px;
            background: transparent;
            border-radius: 6px;
        }
        QMenuBar::item:selected {
            background: @elevated@;
        }
        QMenu {
            background: @panel@;
            color: @textPrimary@;
            border: %1px solid @hairline@;
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
            background: @selection@;
            color: @selectionText@;
        }
        QMenu::separator {
            height: 1px;
            background: @hairline@;
            margin: 4px 8px;
        }
        QToolBar {
            background: @base@;
            border: 0px;
            border-bottom: %1px solid @hairline@;
            spacing: 2px;
            padding: 2px;
        }
        QToolButton, QPushButton {
            min-height: %2px;
            padding: %3px %4px;
            border: %1px solid @hairline@;
            border-radius: 8px;
            background: @elevated@;
            color: @textPrimary@;
        }
        QToolButton:hover, QPushButton:hover {
            background: @elevatedHover@;
            border-color: @border@;
        }
        QToolButton:pressed, QPushButton:pressed {
            background: @selection@;
            border-color: @accent@;
        }
        QToolButton:disabled, QPushButton:disabled {
            color: @textMuted@;
            background: @panel@;
        }
        QTabWidget::pane {
            border-top: %1px solid @hairline@;
            background: @base@;
        }
        QTabBar::tab {
            min-height: %5px;
            padding: 0px %4px;
            margin: 2px 1px 0px 1px;
            border: 0px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            background: transparent;
            color: @textSecondary@;
        }
        QTabBar::tab:hover {
            background: @elevated@;
        }
        QTabBar::tab:selected {
            background: @elevated@;
            color: @selectionText@;
        }
        QLabel#previewPanel {
            background: @base@;
            border: %1px solid @hairline@;
            border-radius: 10px;
            color: @textSecondary@;
        }
        QListWidget#logPanel, QListWidget#problemPanel, QPlainTextEdit#terminalOutput {
            background: @base@;
            border: 0px;
            color: @textPrimary@;
        }
        QListWidget, QTextEdit, QPlainTextEdit, QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            border: %1px solid @hairline@;
            border-radius: 6px;
            background: @input@;
            selection-background-color: @selection@;
            selection-color: @selectionText@;
            color: @textPrimary@;
        }
        QListWidget::item {
            min-height: %2px;
            padding: %1px %4px;
            border-radius: 6px;
        }
        QListWidget::item:hover {
            background: @elevated@;
        }
        QListWidget::item:selected {
            background: @selection@;
            color: @selectionText@;
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
            border-color: @accent@;
            background: @input@;
        }
        QCheckBox, QLabel {
            color: @textPrimary@;
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
            background: @scrollHandle@;
            border-radius: 4px;
            min-height: %2px;
            min-width: %2px;
        }
        QScrollBar::handle:hover {
            background: @scrollHandleHover@;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0px;
            height: 0px;
        }
        QScrollBar::add-page, QScrollBar::sub-page {
            background: transparent;
        }
        QWidget#canvasZoomOverlay {
            background: @overlayBg@;
            border: %1px solid @hairline@;
            border-radius: 12px;
        }
        QToolTip {
            background: @elevated@;
            color: @textPrimary@;
            border: %1px solid @border@;
            border-radius: 8px;
            padding: %3px %4px;
        }
        /* 画布标签条：Chrome 风标签由 CanvasTabBar 自绘，这里只给容器底色 / 底边线。
           strip 用 panel 底色，激活标签（base 色）凸显并与下方 base 内容融合。 */
        QWidget#canvasTabStrip {
            background: @panel@;
            border-bottom: %1px solid @hairline@;
        }
        QWidget#breadcrumbBar {
            background: @base@;
            border-bottom: %1px solid @hairline@;
        }
        QToolButton#canvasNewButton {
            border: 0px;
            border-radius: 7px;
            background: transparent;
            padding: 2px;
        }
        QToolButton#canvasNewButton:hover {
            background: @elevated@;
        }
        QToolButton#breadcrumbNav {
            border: 0px;
            background: transparent;
            border-radius: 6px;
            padding: 2px 5px;
        }
        QToolButton#breadcrumbNav:hover {
            background: @elevated@;
        }
        QToolButton#breadcrumbSegment {
            border: 0px;
            background: transparent;
            border-radius: 6px;
            padding: 1px 7px;
            color: @textSecondary@;
        }
        QToolButton#breadcrumbSegment:hover {
            background: @elevated@;
            color: @textPrimary@;
        }
        QToolButton#breadcrumbCurrent {
            border: 0px;
            background: transparent;
            padding: 1px 7px;
            color: @textPrimary@;
            font-weight: 600;
        }
        QLabel#breadcrumbSep {
            color: @textMuted@;
            padding: 0px 1px;
        }
    )")
        .arg(border)
        .arg(itemHeight)
        .arg(padV)
        .arg(padH)
        .arg(tabHeight);

    const Palette p = palette();
    const std::vector<std::pair<QString, QString>> tokens = {
        {"@base@", p.base.name()},
        {"@panel@", p.panel.name()},
        {"@elevated@", p.elevated.name()},
        {"@elevatedHover@", p.elevatedHover.name()},
        {"@input@", p.input.name()},
        {"@hairline@", p.hairline.name()},
        {"@border@", p.border.name()},
        {"@textPrimary@", p.textPrimary.name()},
        {"@textSecondary@", p.textSecondary.name()},
        {"@textMuted@", p.textMuted.name()},
        {"@accent@", p.accent.name()},
        {"@selection@", p.selection.name()},
        {"@selectionText@", p.selectionText.name()},
        {"@scrollHandle@", p.scrollHandle.name()},
        {"@scrollHandleHover@", p.scrollHandleHover.name()},
        {"@overlayBg@", rgba(QColor(p.panel.red(), p.panel.green(), p.panel.blue(), 235))},
    };
    for (const auto& [key, value] : tokens) {
        sheet.replace(key, value);
    }
    return sheet;
}

ThemePreference themePreference()
{
    return g_themePreference;
}

QString themePreferenceName()
{
    return g_themePreference == ThemePreference::Light ? "light" : "dark";
}

void setThemePreference(const QString& preference)
{
    setThemePreference(preference.compare("light", Qt::CaseInsensitive) == 0
                           ? ThemePreference::Light
                           : ThemePreference::Dark);
}

void setThemePreference(ThemePreference preference)
{
    g_themePreference = preference;
}

bool isDarkTheme()
{
    return g_themePreference == ThemePreference::Dark;
}

void apply(QApplication& app, double uiScale)
{
    app.setFont(appFont(uiScale));
    app.setStyleSheet(styleSheet(uiScale));

    const Palette p = palette();
    QPalette qtPalette = app.palette();
    qtPalette.setColor(QPalette::Window, p.base);
    qtPalette.setColor(QPalette::WindowText, p.textPrimary);
    qtPalette.setColor(QPalette::Button, p.elevated);
    qtPalette.setColor(QPalette::Base, p.base);
    qtPalette.setColor(QPalette::AlternateBase, p.panel);
    qtPalette.setColor(QPalette::Text, p.textPrimary);
    qtPalette.setColor(QPalette::ButtonText, p.textPrimary);
    qtPalette.setColor(QPalette::BrightText, isDarkTheme() ? QColor("#ffffff") : QColor("#000000"));
    qtPalette.setColor(QPalette::Highlight, p.selection);
    qtPalette.setColor(QPalette::HighlightedText, p.selectionText);
    qtPalette.setColor(QPalette::ToolTipBase, p.elevated);
    qtPalette.setColor(QPalette::ToolTipText, p.textPrimary);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    qtPalette.setColor(QPalette::PlaceholderText, p.textMuted);
#endif
    app.setPalette(qtPalette);
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
