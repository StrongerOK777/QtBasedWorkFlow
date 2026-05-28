#pragma once

#include <QWidget>

class ProblemModel;
class QQuickWidget;
class QSplitter;
class QuickAccessModel;
class WorkbenchBridge;
class WorkbenchCommandRegistry;
class NodeCatalogModel;
class WorkflowCheckpointModel;
class WorkflowOutlineModel;
class WorkflowTemplateModel;

class WorkbenchHostWidget final : public QWidget {
    Q_OBJECT
public:
    WorkbenchHostWidget(WorkbenchBridge* bridge,
                        WorkbenchCommandRegistry* commands,
                        NodeCatalogModel* nodes,
                        WorkflowOutlineModel* outline,
                        ProblemModel* problems,
                        WorkflowTemplateModel* templates,
                        WorkflowCheckpointModel* checkpoints,
                        QuickAccessModel* quickAccess,
                        QWidget* editor,
                        QWidget* preview,
                        QWidget* bottomPanel,
                        double uiScale,
                        QWidget* parent = nullptr);

    QSplitter* workbenchSplitter() const { return workbenchSplitter_; }
    QSplitter* editorSplitter() const { return editorSplitter_; }
    QWidget* primarySidebar() const;
    QWidget* previewSidebar() const { return previewSidebar_; }
    QWidget* bottomPanel() const { return bottomPanel_; }
    void showQuickAccess();
    void setUiScale(double uiScale);
    // 在 bridge / 各 Model 被销毁前调用，清空所有 QML 表面，断开桥接连接，
    // 避免退出时 QML 绑定访问悬空对象而报错。
    void teardownSurfaces();

private:
    QQuickWidget* makeQuickSurface(const QUrl& source, QWidget* parent);

    WorkbenchBridge* bridge_ = nullptr;
    WorkbenchCommandRegistry* commands_ = nullptr;
    NodeCatalogModel* nodes_ = nullptr;
    WorkflowOutlineModel* outline_ = nullptr;
    ProblemModel* problems_ = nullptr;
    WorkflowTemplateModel* templates_ = nullptr;
    WorkflowCheckpointModel* checkpoints_ = nullptr;
    QuickAccessModel* quickAccess_ = nullptr;
    QQuickWidget* activitySurface_ = nullptr;
    QQuickWidget* titleSurface_ = nullptr;
    QQuickWidget* editorHeaderSurface_ = nullptr;
    QQuickWidget* sidebarSurface_ = nullptr;
    QQuickWidget* statusSurface_ = nullptr;
    QWidget* quickAccessPopup_ = nullptr;
    QWidget* previewSidebar_ = nullptr;
    QWidget* bottomPanel_ = nullptr;
    QSplitter* workbenchSplitter_ = nullptr;
    QSplitter* editorSplitter_ = nullptr;
    double uiScale_ = 1.0;
};
