#pragma once

#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowGraph.h"

#include <QMainWindow>
#include <QMap>

class QFormLayout;
class QCloseEvent;
class QDockWidget;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QListWidget;
class QTextEdit;
class QWidget;

class MainWindow : public QMainWindow {
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

    WorkflowGraph graph_;
    ExecutionResult lastResult_;
    QString currentFile_;
    QString selectedNodeId_;
    QString pendingFromNode_;
    QString pendingFromPort_;
    double zoomScale_ = 1.0;

    QListWidget* palette_ = nullptr;
    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    QWidget* propertyPanel_ = nullptr;
    QFormLayout* propertyLayout_ = nullptr;
    QLabel* preview_ = nullptr;
    QTextEdit* log_ = nullptr;
    QDockWidget* paletteDock_ = nullptr;
    QDockWidget* propertyDock_ = nullptr;
    QDockWidget* bottomDock_ = nullptr;
    QMap<QString, QGraphicsItem*> nodeItems_;
    QVector<QGraphicsItem*> edgeItems_;
};
