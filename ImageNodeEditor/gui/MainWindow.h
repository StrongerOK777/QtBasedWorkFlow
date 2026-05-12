#pragma once

#include "gui/ElaCompat.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowGraph.h"

#include <QMap>
#include <QVariant>

class QFormLayout;
class QCloseEvent;
class QDockWidget;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QListWidget;
class QTextEdit;
class QToolButton;
class QToolBar;
class QWidget;

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
    void focusParameterPanel();
    void zoomIn();
    void zoomOut();
    void applyZoomFactor(double factor);
    double uiScale() const { return uiScale_; }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void createActions();
    void createLayout();
    void rebuildPalette();
    void rebuildProperties();
    void appendLog(const QString& message);
    void runWorkflow();
    void newWorkflow();
    void openWorkflow();
    void saveWorkflow();
    void saveWorkflowAs();
    bool ensureSavePath();
    void removeEdgeByIndex(int edgeIndex);
    QPointF findAvailableNodePosition(const QPointF& requested) const;
    void setPreviewImage(const QImage& image);
    void updatePreviewForSelection();
    void resetDockLayout();
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
    void focusFailedNode(const QString& nodeId);

    WorkflowGraph graph_;
    ExecutionEngine engine_;
    ExecutionResult lastResult_;
    QString currentFile_;
    QString selectedNodeId_;
    QString pendingFromNode_;
    QString pendingFromPort_;
    double zoomScale_ = 1.0;
    double uiScale_ = 1.0;

    QListWidget* palette_ = nullptr;
    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    QWidget* propertyPanel_ = nullptr;
    QFormLayout* propertyLayout_ = nullptr;
    QLabel* preview_ = nullptr;
    QTextEdit* log_ = nullptr;
    GuiCompat::DockWidget* paletteDock_ = nullptr;
    GuiCompat::DockWidget* propertyDock_ = nullptr;
    GuiCompat::DockWidget* bottomDock_ = nullptr;
    QToolButton* canvasZoomInButton_ = nullptr;
    QToolButton* canvasZoomOutButton_ = nullptr;
    QWidget* canvasZoomOverlay_ = nullptr;
    QToolBar* mainToolbar_ = nullptr;
    QToolBar* layoutToolbar_ = nullptr;
    QToolBar* viewToolbar_ = nullptr;
    QMap<QString, QGraphicsItem*> nodeItems_;
    QMap<QString, NodeExecutionState> nodeRunStates_;
    QVector<QGraphicsItem*> edgeItems_;
};
