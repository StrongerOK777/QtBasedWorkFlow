#pragma once

#include "gui/AppTheme.h"
#include "nodes/NodeFactory.h"

#include <QAbstractListModel>
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVector>

class QAction;
class WorkflowGraph;

// 把活动主题 palette 与界面缩放暴露给 QML 工作台表面。QML 通过 `theme.*` 绑定颜色，
// 通过 `theme.scale` 让字号跟随界面缩放；切换主题或缩放时调用 refresh()/setScale()
// 触发 QML 重新绑定，使深色 / 浅色与放大字体在 QML 侧实时生效。
class WorkbenchTheme final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor base READ base NOTIFY changed)
    Q_PROPERTY(QColor panel READ panel NOTIFY changed)
    Q_PROPERTY(QColor elevated READ elevated NOTIFY changed)
    Q_PROPERTY(QColor elevatedHover READ elevatedHover NOTIFY changed)
    Q_PROPERTY(QColor input READ input NOTIFY changed)
    Q_PROPERTY(QColor hairline READ hairline NOTIFY changed)
    Q_PROPERTY(QColor border READ border NOTIFY changed)
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY changed)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY changed)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY changed)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(QColor accentHover READ accentHover NOTIFY changed)
    Q_PROPERTY(QColor selection READ selection NOTIFY changed)
    Q_PROPERTY(QColor selectionText READ selectionText NOTIFY changed)
    Q_PROPERTY(QColor onAccent READ onAccent NOTIFY changed)
    Q_PROPERTY(QColor warning READ warning NOTIFY changed)
    Q_PROPERTY(QColor danger READ danger NOTIFY changed)
    Q_PROPERTY(double scale READ scale NOTIFY scaleChanged)
public:
    explicit WorkbenchTheme(QObject* parent = nullptr);

    QColor base() const { return palette_.base; }
    QColor panel() const { return palette_.panel; }
    QColor elevated() const { return palette_.elevated; }
    QColor elevatedHover() const { return palette_.elevatedHover; }
    QColor input() const { return palette_.input; }
    QColor hairline() const { return palette_.hairline; }
    QColor border() const { return palette_.border; }
    QColor textPrimary() const { return palette_.textPrimary; }
    QColor textSecondary() const { return palette_.textSecondary; }
    QColor textMuted() const { return palette_.textMuted; }
    QColor accent() const { return palette_.accent; }
    QColor accentHover() const { return palette_.accentHover; }
    QColor selection() const { return palette_.selection; }
    QColor selectionText() const { return palette_.selectionText; }
    QColor onAccent() const { return palette_.onAccent; }
    QColor warning() const { return palette_.warning; }
    QColor danger() const { return palette_.danger; }
    double scale() const { return scale_; }

    void setScale(double scale);
    void refresh();

signals:
    void changed();
    void scaleChanged();

private:
    AppTheme::Palette palette_;
    double scale_ = 1.0;
};

class WorkflowTemplateModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role {
        TemplateIdRole = Qt::UserRole + 1,
        TitleRole,
        DetailRole,
        SourceRole,
        BuiltInRole
    };

    struct Entry {
        QString id;
        QString title;
        QString detail;
        QString source;
        bool builtIn = false;
    };

    explicit WorkflowTemplateModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEntries(QVector<Entry> entries);
    Entry entryById(const QString& id) const;

private:
    QVector<Entry> entries_;
};

class WorkflowCheckpointModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role {
        CheckpointIdRole = Qt::UserRole + 1,
        TitleRole,
        DetailRole,
        BranchRole
    };

    struct Entry {
        QString id;
        QString title;
        QString detail;
        QString branch;
    };

    explicit WorkflowCheckpointModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEntries(QVector<Entry> entries);
    Entry entryById(const QString& id) const;

private:
    QVector<Entry> entries_;
};

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
    Q_PROPERTY(bool windowMaximized READ windowMaximized NOTIFY windowMaximizedChanged)
public:
    WorkbenchBridge(WorkbenchCommandRegistry* commands, QuickAccessModel* quickAccess, QObject* parent = nullptr);

    QString documentTitle() const { return documentTitle_; }
    QString statusText() const { return statusText_; }
    QString selectedNodeText() const { return selectedNodeText_; }
    QString zoomText() const { return zoomText_; }
    QString activeSidebar() const { return activeSidebar_; }
    bool previewVisible() const { return previewVisible_; }
    bool panelVisible() const { return panelVisible_; }
    bool windowMaximized() const { return windowMaximized_; }

    void setDocumentTitle(const QString& title);
    void setStatusText(const QString& status);
    void setSelectedNodeText(const QString& selectedNode);
    void setZoomText(const QString& zoom);
    void setActiveSidebar(const QString& sidebar);
    void setPreviewVisible(bool visible);
    void setPanelVisible(bool visible);
    void setWindowMaximized(bool maximized);

    Q_INVOKABLE void createNode(const QString& typeName);
    Q_INVOKABLE void focusNode(const QString& nodeId);
    Q_INVOKABLE void triggerCommand(const QString& id);
    Q_INVOKABLE void showQuickAccess();
    Q_INVOKABLE void activateQuickAccess(int row);
    Q_INVOKABLE void startNodeDrag(const QString& typeName, const QString& title, const QString& category);
    Q_INVOKABLE void saveWorkflowTemplate();
    Q_INVOKABLE void applyWorkflowTemplate(const QString& templateId);
    Q_INVOKABLE void createCheckpoint();
    Q_INVOKABLE void restoreCheckpoint(const QString& checkpointId);
    Q_INVOKABLE void branchFromCheckpoint(const QString& checkpointId);
    Q_INVOKABLE void restoreTimeline(const QString& timelineId);
    Q_INVOKABLE void requestWindowMove();
    Q_INVOKABLE void requestWindowMinimize();
    Q_INVOKABLE void requestWindowMaximizeToggle();
    Q_INVOKABLE void requestWindowClose();
    Q_INVOKABLE void showTooltip(const QString& text, const QString& placement);
    Q_INVOKABLE void hideTooltip();

signals:
    void documentTitleChanged();
    void statusTextChanged();
    void selectedNodeTextChanged();
    void zoomTextChanged();
    void activeSidebarChanged();
    void previewVisibleChanged();
    void panelVisibleChanged();
    void windowMaximizedChanged();
    void nodeCreationRequested(const QString& typeName);
    void nodeFocusRequested(const QString& nodeId);
    void recentWorkflowRequested(const QString& path);
    void workflowTemplateSaveRequested();
    void workflowTemplateApplyRequested(const QString& templateId);
    void checkpointCreateRequested();
    void checkpointRestoreRequested(const QString& checkpointId);
    void checkpointBranchRequested(const QString& checkpointId);
    void timelineRestoreRequested(const QString& timelineId);
    void previewVisibilityRequested(bool visible);
    void panelVisibilityRequested(bool visible);
    void quickAccessRequested();
    void quickAccessFinished();
    void windowMoveRequested();
    void windowMinimizeRequested();
    void windowMaximizeToggleRequested();
    void windowCloseRequested();
    void tooltipRequested(const QString& text, const QString& placement);
    void tooltipHideRequested();

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
    bool windowMaximized_ = false;
};
