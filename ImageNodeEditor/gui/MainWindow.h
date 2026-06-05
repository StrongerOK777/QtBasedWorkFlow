#pragma once

#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowGraph.h"

#include <QImage>
#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <memory>

class QAction;
class QCloseEvent;
class QFrame;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QSplitter;
class QHBoxLayout;
class QTabBar;
class QTabWidget;
class QThread;
class QToolButton;
class QToolBar;
class QTimer;
class QUndoStack;
class QWidget;
class QShowEvent;
class TerminalPanel;
class WorkbenchBridge;
class WorkbenchTheme;
class WorkflowCheckpointModel;
class WorkbenchCommandRegistry;
class WorkbenchHostWidget;
class NodeCatalogModel;
class ProblemModel;
class QuickAccessModel;
class WorkflowCanvas;
class WorkflowOutlineModel;
class WorkflowTemplateModel;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    WorkflowGraph& graph() { return graph_; }
    void addNodeFromType(const QString& typeName, const QPointF& position = {});
    void selectNode(const QString& nodeId);
    void requestConnect(const QString& fromNode, const QString& fromPort, const QString& toNode, const QString& toPort);
    void updateNodePosition(const QString& nodeId, const QPointF& position);
    void rebuildScene();
    void rebuildEdges();
    void removeSelectedItems();
    void showNodeContextMenu(const QString& nodeId, const QPoint& screenPos);
    void showEdgeContextMenu(int edgeIndex, const QPoint& screenPos);
    void copySelectedNode();
    void showQuickNodePalette();
    void showQuickNodePaletteAt(const QPointF& scenePosition);
    void focusParameterPanel();
    void zoomIn();
    void zoomOut();
    void applyZoomFactor(double factor);
    double uiScale() const { return uiScale_; }
    void restoreGraphFromUndo(const WorkflowGraph& graph, const QString& selectedNodeId);
    void commitNodeMove(const QString& nodeId, const QPointF& before, const QPointF& after);
    void updateMiniMap();
    void enterMacroNode(const QString& nodeId);
    void setNodeParameterForNode(const QString& nodeId, const QString& name, const QVariant& value);

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void createActions();
    void createLayout();
    void rebuildNodeMenus();
    void addNodeFromMenu(const QString& typeName);
    void rebuildProperties();
    bool acceptCanvasEdge(const Edge& edge);
    void removeCanvasEdge(const Edge& edge);
    void appendLog(const QString& message, const QString& nodeId = {});
    void appendProblem(const QString& message, const QString& nodeId = {});
    QString nodeLabel(const QString& nodeId) const;
    void fitGraphInView();
    void clearLog();
    void runWorkflow();
    void exportWorkflowCopy();
    void exportPreviewImage();
    void exportCanvasImage();
    void encapsulateSelectionAsMacro();
    void autoLayoutWorkflow();
    void leaveMacroNode();
    void navigateForwardMacroNode();
    void enterMacroNodeInternal(const QString& nodeId, bool clearForwardHistory);
    WorkflowGraph graphForPersistence() const;
    void newWorkflow();
    void openWorkflow();
    void openWorkflowPath(const QString& path, bool confirmCurrentWorkflow);
    bool saveWorkflow();
    void saveWorkflowAs();
    void rememberRecentWorkflow(const QString& path);
    void refreshRecentWorkflowModel();
    void refreshWorkflowTemplateModel();
    void refreshWorkflowCheckpointModel();
    void refreshWorkflowTimelineModel();
    void recordSaveTimeline();
    void restoreTimelineEntry(const QString& timelineId);
    void saveCurrentWorkflowAsTemplate();
    void applyWorkflowTemplate(const QString& templateId);
    void createWorkflowCheckpoint();
    void restoreWorkflowCheckpoint(const QString& checkpointId);
    void branchFromWorkflowCheckpoint(const QString& checkpointId);
    bool confirmSaveIfNeeded();
    bool ensureSavePath();
    void updateWindowTitle();
    void updateNavigationActions();
    void updateBreadcrumb();
    void navigateToDepth(int targetDepth);
    void pushGraphEditCommand(const QString& text,
                              const WorkflowGraph& before,
                              const QString& selectedBefore,
                              const WorkflowGraph& after,
                              const QString& selectedAfter,
                              const QString& mergeKey = {});
    void removeEdgeByIndex(int edgeIndex);
    QPointF findAvailableNodePosition(const QPointF& requested) const;
    void setPreviewImage(const QImage& image);
    void updatePreviewForSelection();
    void resetWorkbenchLayout();
    void toggleFullScreenMode();
    QMenu* createCanvasContextMenu(const QPointF& scenePosition);
    void showSettingsDialog();
    void increaseUiScale();
    void decreaseUiScale();
    void resetUiScale();
    void setUiScale(double scale);
    void applyUiScale();
    void setSelectedNodeParameter(const QString& name, const QVariant& value);
    void resetNodeRunStates();
    void applyNodeRunState(const QString& nodeId, NodeExecutionState state);
    void handleNodeExecutionEvent(const NodeExecutionSummary& summary);
    void setExecutionBusy(bool busy);
    bool rejectGraphReplacementWhileBusy(const QString& actionName);
    void finishWorkflowExecution(int generation,
                                 const Result<ExecutionResult>& result,
                                 const ExecutionResult& lastResult,
                                 const ExecutionEngine& engine);
    void finishLivePreview(int generation,
                           const QString& previewNodeId,
                           const Result<ExecutionResult>& result,
                           const ExecutionResult& lastResult,
                           const ExecutionEngine& engine);
    void trackWorkerThread(QThread* thread);
    void showWorkbenchTooltip(const QString& text, const QString& placement);
    void hideWorkbenchTooltip();
    void updateRunAnimation();
    void focusFailedNode(const QString& nodeId);
    void focusLogNode(QListWidgetItem* item);
    void highlightNodeBriefly(const QString& nodeId);
    void scheduleLivePreview();
    void runLivePreview();
    void initializeWorkbook();
    void syncCurrentWorkbookPage();
    void switchWorkbookPage(int index);
    void addWorkbookPage();
    void closeWorkbookPage(int index);
    void refreshWorkbookTabs();
    bool currentWorkbookDirty() const;

    WorkflowGraph graph_;
    struct GraphContext {
        WorkflowGraph graph;
        QString macroNodeId;
        QString selectedNodeId;
    };
    QVector<GraphContext> graphStack_;
    QStringList forwardMacroHistory_;
    ExecutionEngine engine_;
    ExecutionResult lastResult_;
    QString currentFile_;
    QString selectedNodeId_;
    QString pendingFromNode_;
    QString pendingFromPort_;
    double zoomScale_ = 1.0;
    double wheelZoomStep_ = 1.04;
    double uiScale_ = 1.0;
    struct WorkbookPage {
        WorkflowGraph graph;
        QString selectedNodeId;
        QString filePath;
        QString title;
        bool dirty = false;
    };
    QVector<WorkbookPage> workbookPages_;
    int currentWorkbookIndex_ = -1;
    bool switchingWorkbook_ = false;
    // 进入关闭/析构流程后置位，退出期跳过会访问原生窗口/QML 桥的回调，避免崩溃。
    bool shuttingDown_ = false;

    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    std::unique_ptr<WorkflowCanvas> canvas_;
    QLabel* preview_ = nullptr;
    QListWidget* log_ = nullptr;
    QListWidget* problemLog_ = nullptr;
    QTabWidget* bottomTabs_ = nullptr;
    TerminalPanel* terminalPanel_ = nullptr;
    WorkbenchHostWidget* workbenchHost_ = nullptr;
    WorkbenchCommandRegistry* workbenchCommands_ = nullptr;
    NodeCatalogModel* nodeCatalogModel_ = nullptr;
    WorkflowOutlineModel* workflowOutlineModel_ = nullptr;
    ProblemModel* problemModel_ = nullptr;
    QuickAccessModel* quickAccessModel_ = nullptr;
    WorkflowTemplateModel* workflowTemplateModel_ = nullptr;
    WorkflowCheckpointModel* workflowCheckpointModel_ = nullptr;
    WorkflowCheckpointModel* workflowTimelineModel_ = nullptr;
    WorkbenchBridge* workbenchBridge_ = nullptr;
    WorkbenchTheme* workbenchTheme_ = nullptr;
    QWidget* primarySidebar_ = nullptr;
    QWidget* previewSidebar_ = nullptr;
    QWidget* bottomPanel_ = nullptr;
    QWidget* activityBar_ = nullptr;
    QListWidget* workflowList_ = nullptr;
    QSplitter* workbenchSplitter_ = nullptr;
    QSplitter* editorSplitter_ = nullptr;
    QToolButton* canvasZoomInButton_ = nullptr;
    QToolButton* canvasZoomOutButton_ = nullptr;
    QWidget* canvasZoomOverlay_ = nullptr;
    QToolBar* mainToolbar_ = nullptr;
    QToolBar* headerToolbar_ = nullptr;
    QToolBar* workbookToolbar_ = nullptr;
    QTabBar* workbookTabs_ = nullptr;
    QWidget* breadcrumbBar_ = nullptr;
    QHBoxLayout* breadcrumbPathLayout_ = nullptr;
    QLabel* documentTitleLabel_ = nullptr;
    QMenu* fileMenu_ = nullptr;
    QMenu* editMenu_ = nullptr;
    QMenu* nodeOperationMenu_ = nullptr;
    QMenu* viewMenu_ = nullptr;
    QMenu* settingsMenu_ = nullptr;
    QMenu* layoutMenu_ = nullptr;
    QMenu* commandCenterMenu_ = nullptr;
    QAction* commandCenterAction_ = nullptr;
    QAction* newWorkbookAction_ = nullptr;
    QAction* returnToParentAction_ = nullptr;
    QAction* forwardToChildAction_ = nullptr;
    QAction* runAction_ = nullptr;
    QAction* exportWorkflowCopyAction_ = nullptr;
    QAction* exportPreviewAction_ = nullptr;
    QAction* settingsAction_ = nullptr;
    QAction* previewToggleAction_ = nullptr;
    QAction* bottomToggleAction_ = nullptr;
    QWidget* miniMap_ = nullptr;
    QString currentWorkflowBranch_ = "main";
    QTimer* livePreviewTimer_ = nullptr;
    QTimer* runAnimationTimer_ = nullptr;
    QVector<QThread*> workerThreads_;
    bool executionBusy_ = false;
    int executionGeneration_ = 0;
    int livePreviewGeneration_ = 0;
    QFrame* tooltipPopup_ = nullptr;
    QLabel* tooltipLabel_ = nullptr;
    QUndoStack* undoStack_ = nullptr;
    QMap<QString, QGraphicsItem*> nodeItems_;
    QMap<QString, NodeExecutionState> nodeRunStates_;
    QMap<QString, qint64> nodeElapsedMs_;
    int runAnimationPhase_ = 0;
    QImage currentPreviewImage_;
};
