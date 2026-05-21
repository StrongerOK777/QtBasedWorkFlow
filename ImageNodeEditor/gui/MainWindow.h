#pragma once

#include "gui/ElaCompat.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowGraph.h"

#include <QImage>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QVector>

class QAction;
class QCloseEvent;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QSplitter;
class QTabBar;
class QTabWidget;
class QToolButton;
class QToolBar;
class QTimer;
class QUndoStack;
class QWidget;
class TerminalPanel;

class MainWindow : public GuiCompat::MainWindowBase {
public:
    explicit MainWindow(QWidget* parent = nullptr);

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

private:
    void createActions();
    void createLayout();
    void rebuildNodeMenus();
    void addNodeFromMenu(const QString& typeName);
    void rebuildProperties();
    void appendLog(const QString& message, const QString& nodeId = {});
    void appendProblem(const QString& message, const QString& nodeId = {});
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
    bool saveWorkflow();
    void saveWorkflowAs();
    bool confirmSaveIfNeeded();
    bool ensureSavePath();
    void updateWindowTitle();
    void updateNavigationActions();
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

    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    QLabel* preview_ = nullptr;
    QListWidget* log_ = nullptr;
    QListWidget* problemLog_ = nullptr;
    QTabWidget* bottomTabs_ = nullptr;
    TerminalPanel* terminalPanel_ = nullptr;
    QWidget* primarySidebar_ = nullptr;
    QWidget* previewSidebar_ = nullptr;
    QWidget* bottomPanel_ = nullptr;
    QWidget* activityBar_ = nullptr;
    QWidget* nodeLibraryPage_ = nullptr;
    QWidget* workflowPage_ = nullptr;
    QListWidget* nodeLibraryList_ = nullptr;
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
    QTimer* livePreviewTimer_ = nullptr;
    QTimer* runAnimationTimer_ = nullptr;
    QUndoStack* undoStack_ = nullptr;
    QMap<QString, QGraphicsItem*> nodeItems_;
    QMap<QString, NodeExecutionState> nodeRunStates_;
    QMap<QString, qint64> nodeElapsedMs_;
    int runAnimationPhase_ = 0;
    QImage currentPreviewImage_;
    QVector<QGraphicsItem*> edgeItems_;
};
