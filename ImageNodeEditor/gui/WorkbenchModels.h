#pragma once

#include "nodes/NodeFactory.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QStringList>
#include <QVector>

class QAction;
class WorkflowGraph;

class WorkbenchCommandRegistry final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        CategoryRole,
        ShortcutRole,
        EnabledRole
    };

    struct Entry {
        QString id;
        QString title;
        QString category;
        QString shortcut;
        QPointer<QAction> action;
    };

    explicit WorkbenchCommandRegistry(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addAction(const QString& id, const QString& category, QAction* action);
    QVector<Entry> entries() const { return entries_; }
    Q_INVOKABLE bool trigger(const QString& id);

private:
    QVector<Entry> entries_;
};

class NodeCatalogModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
public:
    enum Role {
        TypeNameRole = Qt::UserRole + 1,
        TitleRole,
        CategoryRole,
        ProviderRole,
        IconNameRole
    };

    explicit NodeCatalogModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filterText() const { return filterText_; }
    void setFilterText(const QString& filterText);
    QVector<NodeDescriptor> descriptors() const { return descriptors_; }

signals:
    void filterTextChanged();

private:
    void refilter();

    QVector<NodeDescriptor> descriptors_;
    QVector<int> filteredRows_;
    QString filterText_;
};

class WorkflowOutlineModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
public:
    enum Role {
        NodeIdRole = Qt::UserRole + 1,
        TitleRole,
        TypeNameRole,
        CategoryRole
    };

    struct Entry {
        QString nodeId;
        QString title;
        QString typeName;
        QString category;
    };

    explicit WorkflowOutlineModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setGraph(const WorkflowGraph& graph);
    QString filterText() const { return filterText_; }
    void setFilterText(const QString& filterText);
    QVector<Entry> entries() const { return entries_; }

signals:
    void filterTextChanged();

private:
    void refilter();

    QVector<Entry> entries_;
    QVector<int> filteredRows_;
    QString filterText_;
};

class ProblemModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role {
        MessageRole = Qt::UserRole + 1,
        NodeIdRole
    };

    struct Entry {
        QString message;
        QString nodeId;
    };

    explicit ProblemModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void append(const QString& message, const QString& nodeId);
    void clear();
    QVector<Entry> entries() const { return entries_; }

private:
    QVector<Entry> entries_;
};

class QuickAccessModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
public:
    enum Role {
        KindRole = Qt::UserRole + 1,
        IdRole,
        TitleRole,
        DetailRole
    };

    enum class Kind {
        Command,
        Node,
        WorkflowNode,
        Problem,
        RecentWorkflow
    };

    struct Entry {
        Kind kind = Kind::Command;
        QString id;
        QString title;
        QString detail;
        QString typeName;
        QString nodeId;
        QString path;
    };

    QuickAccessModel(WorkbenchCommandRegistry* commands,
                     NodeCatalogModel* nodeCatalog,
                     WorkflowOutlineModel* workflowOutline,
                     ProblemModel* problems,
                     QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString query() const { return query_; }
    void setQuery(const QString& query);
    void setRecentWorkflowPaths(const QStringList& paths);
    Entry entryAt(int row) const;
    Q_INVOKABLE void refresh();

signals:
    void queryChanged();

private:
    WorkbenchCommandRegistry* commands_ = nullptr;
    NodeCatalogModel* nodeCatalog_ = nullptr;
    WorkflowOutlineModel* workflowOutline_ = nullptr;
    ProblemModel* problems_ = nullptr;
    QString query_;
    QStringList recentWorkflowPaths_;
    QVector<Entry> entries_;
};

class WorkbenchBridge final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle NOTIFY documentTitleChanged)
    Q_PROPERTY(QString statusText READ statusText WRITE setStatusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString selectedNodeText READ selectedNodeText WRITE setSelectedNodeText NOTIFY selectedNodeTextChanged)
    Q_PROPERTY(QString zoomText READ zoomText WRITE setZoomText NOTIFY zoomTextChanged)
    Q_PROPERTY(QString activeSidebar READ activeSidebar WRITE setActiveSidebar NOTIFY activeSidebarChanged)
    Q_PROPERTY(bool previewVisible READ previewVisible WRITE setPreviewVisible NOTIFY previewVisibleChanged)
    Q_PROPERTY(bool panelVisible READ panelVisible WRITE setPanelVisible NOTIFY panelVisibleChanged)
public:
    WorkbenchBridge(WorkbenchCommandRegistry* commands, QuickAccessModel* quickAccess, QObject* parent = nullptr);

    QString documentTitle() const { return documentTitle_; }
    QString statusText() const { return statusText_; }
    QString selectedNodeText() const { return selectedNodeText_; }
    QString zoomText() const { return zoomText_; }
    QString activeSidebar() const { return activeSidebar_; }
    bool previewVisible() const { return previewVisible_; }
    bool panelVisible() const { return panelVisible_; }

    void setDocumentTitle(const QString& title);
    void setStatusText(const QString& status);
    void setSelectedNodeText(const QString& selectedNode);
    void setZoomText(const QString& zoom);
    void setActiveSidebar(const QString& sidebar);
    void setPreviewVisible(bool visible);
    void setPanelVisible(bool visible);

    Q_INVOKABLE void createNode(const QString& typeName);
    Q_INVOKABLE void focusNode(const QString& nodeId);
    Q_INVOKABLE void triggerCommand(const QString& id);
    Q_INVOKABLE void showQuickAccess();
    Q_INVOKABLE void activateQuickAccess(int row);
    Q_INVOKABLE void startNodeDrag(const QString& typeName, const QString& title, const QString& category);
    Q_INVOKABLE void requestWindowMove();
    Q_INVOKABLE void requestWindowMinimize();
    Q_INVOKABLE void requestWindowMaximizeToggle();
    Q_INVOKABLE void requestWindowClose();

signals:
    void documentTitleChanged();
    void statusTextChanged();
    void selectedNodeTextChanged();
    void zoomTextChanged();
    void activeSidebarChanged();
    void previewVisibleChanged();
    void panelVisibleChanged();
    void nodeCreationRequested(const QString& typeName);
    void nodeFocusRequested(const QString& nodeId);
    void recentWorkflowRequested(const QString& path);
    void previewVisibilityRequested(bool visible);
    void panelVisibilityRequested(bool visible);
    void quickAccessRequested();
    void quickAccessFinished();
    void windowMoveRequested();
    void windowMinimizeRequested();
    void windowMaximizeToggleRequested();
    void windowCloseRequested();

private:
    WorkbenchCommandRegistry* commands_ = nullptr;
    QuickAccessModel* quickAccess_ = nullptr;
    QString documentTitle_ = "未命名";
    QString statusText_ = "就绪";
    QString selectedNodeText_ = "未选择节点";
    QString zoomText_ = "100%";
    QString activeSidebar_ = "nodes";
    bool previewVisible_ = true;
    bool panelVisible_ = true;
};
