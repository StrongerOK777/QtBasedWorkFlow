#include "gui/WorkbenchHostWidget.h"

#include "gui/AppTheme.h"
#include "gui/WorkbenchModels.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QQuickWidget>
#include <QQmlContext>
#include <QResizeEvent>
#include <QSplitter>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

namespace {

constexpr auto kActivitySource = "qrc:/workbench/WorkbenchActivityBar.qml";
constexpr auto kTitleSource = "qrc:/workbench/WorkbenchTitleBar.qml";
constexpr auto kSidebarSource = "qrc:/workbench/WorkbenchSidebar.qml";
constexpr auto kStatusSource = "qrc:/workbench/WorkbenchStatusBar.qml";
constexpr auto kQuickAccessSource = "qrc:/workbench/WorkbenchQuickAccess.qml";

// 透明遮罩：覆盖命令面板以外的区域，单击即关闭面板。置于各 QQuickWidget 表面之上，
// 因此能稳定接收点击，不受 QQuickWidget 事件投递细节影响。
class ClickScrim final : public QWidget {
public:
    explicit ClickScrim(QWidget* parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet("background: transparent;");
    }
    std::function<void()> onPress;

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (onPress) {
            onPress();
        }
        event->accept();
    }
};

}

WorkbenchHostWidget::WorkbenchHostWidget(WorkbenchBridge* bridge,
                                         WorkbenchCommandRegistry* commands,
                                         NodeCatalogModel* nodes,
                                         WorkflowOutlineModel* outline,
                                         ProblemModel* problems,
                                         WorkflowTemplateModel* templates,
                                         WorkflowCheckpointModel* checkpoints,
                                         WorkflowCheckpointModel* timeline,
                                         QuickAccessModel* quickAccess,
                                         QWidget* editor,
                                         QWidget* preview,
                                         QWidget* bottomPanel,
                                         double uiScale,
                                         QWidget* parent)
    : QWidget(parent),
      bridge_(bridge),
      commands_(commands),
      nodes_(nodes),
      outline_(outline),
      problems_(problems),
      templates_(templates),
      checkpoints_(checkpoints),
      timeline_(timeline),
      quickAccess_(quickAccess),
      bottomPanel_(bottomPanel),
      uiScale_(uiScale)
{
    setObjectName("workbenchHost");
    setAttribute(Qt::WA_StyledBackground, true);

    titleSurface_ = makeQuickSurface(QUrl(kTitleSource), this);
    activitySurface_ = makeQuickSurface(QUrl(kActivitySource), this);
    sidebarSurface_ = makeQuickSurface(QUrl(kSidebarSource), this);
    statusSurface_ = makeQuickSurface(QUrl(kStatusSource), this);

    // 编辑器区（画布标签条 + 面包屑 + 画布）由 MainWindow 组装后作为 editor 传入；
    // 不再单独叠一个 QML 编辑器头层（曾在画布顶部渲染出黑条）。
    auto* editorShell = new QWidget;
    editorShell->setObjectName("editorShell");
    editorShell->setAttribute(Qt::WA_StyledBackground, true);
    auto* editorShellLayout = new QVBoxLayout(editorShell);
    editorShellLayout->setContentsMargins(0, 0, 0, 0);
    editorShellLayout->setSpacing(0);
    editorShellLayout->addWidget(editor, 1);

    editorSplitter_ = new QSplitter(Qt::Vertical);
    editorSplitter_->setObjectName("editorSplitter");
    editorSplitter_->setChildrenCollapsible(false);
    editorSplitter_->addWidget(editorShell);
    editorSplitter_->addWidget(bottomPanel_);
    editorSplitter_->setStretchFactor(0, 5);
    editorSplitter_->setStretchFactor(1, 2);

    previewSidebar_ = new QWidget;
    previewSidebar_->setObjectName("previewSidebar");
    previewSidebar_->setAttribute(Qt::WA_StyledBackground, true);
    auto* previewLayout = new QVBoxLayout(previewSidebar_);
    previewLayout->setContentsMargins(AppTheme::px(10, uiScale_), AppTheme::px(10, uiScale_),
                                      AppTheme::px(10, uiScale_), AppTheme::px(10, uiScale_));
    previewLayout->setSpacing(AppTheme::px(8, uiScale_));
    auto* previewTitle = new QLabel("预览");
    previewTitle->setObjectName("workbenchPanelTitle");
    previewLayout->addWidget(previewTitle);
    previewLayout->addWidget(preview, 1);

    workbenchSplitter_ = new QSplitter(Qt::Horizontal);
    workbenchSplitter_->setObjectName("workbenchSplitter");
    workbenchSplitter_->setChildrenCollapsible(false);
    workbenchSplitter_->addWidget(sidebarSurface_);
    workbenchSplitter_->addWidget(editorSplitter_);
    workbenchSplitter_->addWidget(previewSidebar_);
    workbenchSplitter_->setStretchFactor(0, 0);
    workbenchSplitter_->setStretchFactor(1, 1);
    workbenchSplitter_->setStretchFactor(2, 0);

    auto* row = new QWidget;
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);
    rowLayout->addWidget(activitySurface_);
    rowLayout->addWidget(workbenchSplitter_, 1);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(titleSurface_);
    root->addWidget(row, 1);
    root->addWidget(statusSurface_);

    auto* popupFrame = new QFrame(this);
    popupFrame->setObjectName("quickAccessPopup");
    popupFrame->setAttribute(Qt::WA_StyledBackground, true);
    // 容器透明，只显示 QML 内部的圆角卡片（避免方角底色露在圆角之外）。
    popupFrame->setStyleSheet("QFrame#quickAccessPopup { background: transparent; border: 0px; }");
    auto* popupLayout = new QVBoxLayout(popupFrame);
    popupLayout->setContentsMargins(0, 0, 0, 0);
    popupLayout->addWidget(makeQuickSurface(QUrl(kQuickAccessSource), popupFrame));
    quickAccessPopup_ = popupFrame;
    quickAccessPopup_->hide();

    // 命令面板的「点击外部关闭」遮罩层：透明、覆盖整个工作台、置于面板之下。
    auto* scrim = new ClickScrim(this);
    scrim->onPress = [this] {
        if (bridge_) {
            bridge_->activateQuickAccess(-1);  // 与 Esc 一致：触发 quickAccessFinished
        } else if (quickAccessPopup_) {
            quickAccessPopup_->hide();
        }
    };
    quickAccessScrim_ = scrim;
    quickAccessScrim_->hide();

    if (bridge_) {
        connect(bridge_, &WorkbenchBridge::quickAccessRequested, this, &WorkbenchHostWidget::showQuickAccess);
        connect(bridge_, &WorkbenchBridge::quickAccessFinished, quickAccessPopup_, &QWidget::hide);
        connect(bridge_, &WorkbenchBridge::quickAccessFinished, quickAccessScrim_, &QWidget::hide);
    }
    setUiScale(uiScale_);
}

QWidget* WorkbenchHostWidget::primarySidebar() const
{
    return sidebarSurface_;
}

void WorkbenchHostWidget::showQuickAccess()
{
    if (!quickAccessPopup_) {
        return;
    }
    if (quickAccess_) {
        quickAccess_->setQuery({});
    }
    // 先铺满遮罩层并置顶，再把面板叠在遮罩之上：点面板外即落到遮罩 -> 关闭。
    if (quickAccessScrim_) {
        quickAccessScrim_->setGeometry(rect());
        quickAccessScrim_->show();
        quickAccessScrim_->raise();
    }
    const int width = std::min(AppTheme::px(720, uiScale_), std::max(AppTheme::px(440, uiScale_), this->width() - AppTheme::px(80, uiScale_)));
    quickAccessPopup_->resize(width, AppTheme::px(430, uiScale_));
    const QPoint topLeft(std::max(0, (this->width() - width) / 2), AppTheme::px(32, uiScale_));
    quickAccessPopup_->move(topLeft);
    quickAccessPopup_->show();
    quickAccessPopup_->raise();
    if (auto* surface = quickAccessPopup_->findChild<QQuickWidget*>()) {
        surface->setFocus(Qt::PopupFocusReason);
    }
}

void WorkbenchHostWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (quickAccessScrim_ && quickAccessScrim_->isVisible()) {
        quickAccessScrim_->setGeometry(rect());
        if (quickAccessPopup_) {
            quickAccessPopup_->raise();
        }
    }
}

void WorkbenchHostWidget::teardownSurfaces()
{
    if (quickAccessPopup_) {
        quickAccessPopup_->hide();
    }
    if (quickAccessScrim_) {
        quickAccessScrim_->hide();
    }
    if (bridge_) {
        disconnect(bridge_, nullptr, this, nullptr);
        if (quickAccessPopup_) {
            disconnect(bridge_, nullptr, quickAccessPopup_, nullptr);
        }
        if (quickAccessScrim_) {
            disconnect(bridge_, nullptr, quickAccessScrim_, nullptr);
        }
    }
    // 清空每个 QQuickWidget 的源，销毁 QML 组件树及其对 bridge / 各 Model
    // 上下文属性的绑定，确保后续这些对象析构时不会被 QML 再次访问。
    const auto surfaces = findChildren<QQuickWidget*>();
    for (auto* surface : surfaces) {
        surface->setSource(QUrl());
    }
}

void WorkbenchHostWidget::setUiScale(double uiScale)
{
    uiScale_ = uiScale;
    if (activitySurface_) {
        activitySurface_->setFixedWidth(AppTheme::px(48, uiScale_));
    }
    if (titleSurface_) {
        titleSurface_->setFixedHeight(AppTheme::px(35, uiScale_));
    }
    if (sidebarSurface_) {
        sidebarSurface_->setMinimumWidth(AppTheme::px(236, uiScale_));
    }
    if (previewSidebar_) {
        previewSidebar_->setMinimumWidth(AppTheme::px(300, uiScale_));
    }
    if (statusSurface_) {
        statusSurface_->setFixedHeight(AppTheme::px(24, uiScale_));
    }
}

QQuickWidget* WorkbenchHostWidget::makeQuickSurface(const QUrl& source, QWidget* parent)
{
    auto* surface = new QQuickWidget(parent);
    surface->setResizeMode(QQuickWidget::SizeRootObjectToView);
    surface->setClearColor(Qt::transparent);
    surface->setAttribute(Qt::WA_AlwaysStackOnTop, false);
    surface->rootContext()->setContextProperty("workbenchBridge", bridge_);
    surface->rootContext()->setContextProperty("commandRegistry", commands_);
    surface->rootContext()->setContextProperty("nodeCatalogModel", nodes_);
    surface->rootContext()->setContextProperty("workflowOutlineModel", outline_);
    surface->rootContext()->setContextProperty("problemModel", problems_);
    surface->rootContext()->setContextProperty("workflowTemplateModel", templates_);
    surface->rootContext()->setContextProperty("workflowCheckpointModel", checkpoints_);
    surface->rootContext()->setContextProperty("workflowTimelineModel", timeline_);
    surface->rootContext()->setContextProperty("quickAccessModel", quickAccess_);
    surface->setSource(source);
    return surface;
}
