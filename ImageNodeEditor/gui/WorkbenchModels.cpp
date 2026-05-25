#include "gui/WorkbenchModels.h"

#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"

#include <QAction>
#include <QApplication>
#include <QDrag>
#include <QFileInfo>
#include <QKeySequence>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QWidget>
#include <algorithm>

namespace {

bool matchesTerms(const QString& text, const QString& query)
{
    const QString normalized = text.toLower();
    const QStringList terms = query.trimmed().toLower().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString& term : terms) {
        if (!normalized.contains(term)) {
            return false;
        }
    }
    return true;
}

QString categoryForType(const QString& typeName)
{
    for (const auto& descriptor : NodeFactory::instance().descriptors()) {
        if (descriptor.typeName == typeName) {
            return descriptor.category;
        }
    }
    return QStringLiteral("高级功能");
}

QString iconNameForCategory(const QString& category)
{
    if (category == QStringLiteral("输入输出")) {
        return QStringLiteral("files");
    }
    if (category == QStringLiteral("几何变换")) {
        return QStringLiteral("layout");
    }
    if (category == QStringLiteral("色彩处理")) {
        return QStringLiteral("symbol-color");
    }
    if (category == QStringLiteral("滤波处理")) {
        return QStringLiteral("filter");
    }
    if (category == QStringLiteral("合成处理")) {
        return QStringLiteral("git-merge");
    }
    return QStringLiteral("symbol-misc");
}

QString kindName(QuickAccessModel::Kind kind)
{
    switch (kind) {
    case QuickAccessModel::Kind::Command:
        return QStringLiteral("命令");
    case QuickAccessModel::Kind::Node:
        return QStringLiteral("节点");
    case QuickAccessModel::Kind::WorkflowNode:
        return QStringLiteral("工作流");
    case QuickAccessModel::Kind::Problem:
        return QStringLiteral("问题");
    case QuickAccessModel::Kind::RecentWorkflow:
        return QStringLiteral("最近");
    }
    return {};
}

}

WorkflowTemplateModel::WorkflowTemplateModel(QObject* parent) : QAbstractListModel(parent) {}

int WorkflowTemplateModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : entries_.size();
}

QVariant WorkflowTemplateModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(index.row());
    switch (role) {
    case TemplateIdRole:
        return entry.id;
    case TitleRole:
        return entry.title;
    case DetailRole:
        return entry.detail;
    case SourceRole:
        return entry.source;
    case BuiltInRole:
        return entry.builtIn;
    default:
        return {};
    }
}

QHash<int, QByteArray> WorkflowTemplateModel::roleNames() const
{
    return {{TemplateIdRole, "templateId"},
            {TitleRole, "title"},
            {DetailRole, "detail"},
            {SourceRole, "source"},
            {BuiltInRole, "builtIn"}};
}

void WorkflowTemplateModel::setEntries(QVector<Entry> entries)
{
    beginResetModel();
    entries_ = std::move(entries);
    endResetModel();
}

WorkflowTemplateModel::Entry WorkflowTemplateModel::entryById(const QString& id) const
{
    for (const Entry& entry : entries_) {
        if (entry.id == id) {
            return entry;
        }
    }
    return {};
}

WorkflowCheckpointModel::WorkflowCheckpointModel(QObject* parent) : QAbstractListModel(parent) {}

int WorkflowCheckpointModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : entries_.size();
}

QVariant WorkflowCheckpointModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(index.row());
    switch (role) {
    case CheckpointIdRole:
        return entry.id;
    case TitleRole:
        return entry.title;
    case DetailRole:
        return entry.detail;
    case BranchRole:
        return entry.branch;
    default:
        return {};
    }
}

QHash<int, QByteArray> WorkflowCheckpointModel::roleNames() const
{
    return {{CheckpointIdRole, "checkpointId"},
            {TitleRole, "title"},
            {DetailRole, "detail"},
            {BranchRole, "branch"}};
}

void WorkflowCheckpointModel::setEntries(QVector<Entry> entries)
{
    beginResetModel();
    entries_ = std::move(entries);
    endResetModel();
}

WorkflowCheckpointModel::Entry WorkflowCheckpointModel::entryById(const QString& id) const
{
    for (const Entry& entry : entries_) {
        if (entry.id == id) {
            return entry;
        }
    }
    return {};
}

WorkbenchCommandRegistry::WorkbenchCommandRegistry(QObject* parent) : QAbstractListModel(parent) {}

int WorkbenchCommandRegistry::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : entries_.size();
}

QVariant WorkbenchCommandRegistry::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(index.row());
    switch (role) {
    case IdRole:
        return entry.id;
    case TitleRole:
        return entry.title;
    case CategoryRole:
        return entry.category;
    case ShortcutRole:
        return entry.shortcut;
    case EnabledRole:
        return entry.action && entry.action->isEnabled();
    default:
        return {};
    }
}

QHash<int, QByteArray> WorkbenchCommandRegistry::roleNames() const
{
    return {{IdRole, "commandId"},
            {TitleRole, "title"},
            {CategoryRole, "category"},
            {ShortcutRole, "shortcut"},
            {EnabledRole, "enabled"}};
}

void WorkbenchCommandRegistry::addAction(const QString& id, const QString& category, QAction* action)
{
    if (!action || id.trimmed().isEmpty()) {
        return;
    }
    for (const Entry& entry : std::as_const(entries_)) {
        if (entry.id == id) {
            return;
        }
    }
    const int row = entries_.size();
    beginInsertRows({}, row, row);
    entries_.append(Entry{id, action->text(), category, action->shortcut().toString(QKeySequence::NativeText), action});
    endInsertRows();
    connect(action, &QAction::changed, this, [this, action] {
        for (int row = 0; row < entries_.size(); ++row) {
            if (entries_.at(row).action == action) {
                entries_[row].title = action->text();
                entries_[row].shortcut = action->shortcut().toString(QKeySequence::NativeText);
                Q_EMIT dataChanged(index(row), index(row));
                return;
            }
        }
    });
}

bool WorkbenchCommandRegistry::trigger(const QString& id)
{
    for (const Entry& entry : std::as_const(entries_)) {
        if (entry.id == id && entry.action && entry.action->isEnabled()) {
            entry.action->trigger();
            return true;
        }
    }
    return false;
}

NodeCatalogModel::NodeCatalogModel(QObject* parent) : QAbstractListModel(parent)
{
    descriptors_ = NodeFactory::instance().descriptors();
    refilter();
}

int NodeCatalogModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : filteredRows_.size();
}

QVariant NodeCatalogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= filteredRows_.size()) {
        return {};
    }
    const NodeDescriptor& descriptor = descriptors_.at(filteredRows_.at(index.row()));
    switch (role) {
    case TypeNameRole:
        return descriptor.typeName;
    case TitleRole:
        return descriptor.displayName;
    case CategoryRole:
        return descriptor.category;
    case ProviderRole:
        return QStringLiteral("内置节点");
    case IconNameRole:
        return iconNameForCategory(descriptor.category);
    default:
        return {};
    }
}

QHash<int, QByteArray> NodeCatalogModel::roleNames() const
{
    return {{TypeNameRole, "typeName"},
            {TitleRole, "title"},
            {CategoryRole, "category"},
            {ProviderRole, "provider"},
            {IconNameRole, "iconName"}};
}

void NodeCatalogModel::setFilterText(const QString& filterText)
{
    if (filterText_ == filterText) {
        return;
    }
    filterText_ = filterText;
    refilter();
    Q_EMIT filterTextChanged();
}

void NodeCatalogModel::refilter()
{
    beginResetModel();
    filteredRows_.clear();
    for (int row = 0; row < descriptors_.size(); ++row) {
        const auto& descriptor = descriptors_.at(row);
        if (matchesTerms(QString("%1 %2 %3").arg(descriptor.displayName, descriptor.category, descriptor.typeName), filterText_)) {
            filteredRows_.append(row);
        }
    }
    endResetModel();
}

WorkflowOutlineModel::WorkflowOutlineModel(QObject* parent) : QAbstractListModel(parent) {}

int WorkflowOutlineModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : filteredRows_.size();
}

QVariant WorkflowOutlineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= filteredRows_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(filteredRows_.at(index.row()));
    switch (role) {
    case NodeIdRole:
        return entry.nodeId;
    case TitleRole:
        return entry.title;
    case TypeNameRole:
        return entry.typeName;
    case CategoryRole:
        return entry.category;
    default:
        return {};
    }
}

QHash<int, QByteArray> WorkflowOutlineModel::roleNames() const
{
    return {{NodeIdRole, "nodeId"},
            {TitleRole, "title"},
            {TypeNameRole, "typeName"},
            {CategoryRole, "category"}};
}

void WorkflowOutlineModel::setGraph(const WorkflowGraph& graph)
{
    beginResetModel();
    entries_.clear();
    for (const auto& record : graph.nodes()) {
        if (!record.node) {
            continue;
        }
        entries_.append(Entry{record.id,
                              record.node->displayName(),
                              record.node->typeName(),
                              categoryForType(record.node->typeName())});
    }
    std::sort(entries_.begin(), entries_.end(), [](const Entry& first, const Entry& second) {
        return QString::localeAwareCompare(first.title, second.title) < 0;
    });
    filteredRows_.clear();
    for (int row = 0; row < entries_.size(); ++row) {
        if (matchesTerms(QString("%1 %2 %3 %4").arg(entries_.at(row).title,
                                                     entries_.at(row).typeName,
                                                     entries_.at(row).category,
                                                     entries_.at(row).nodeId),
                         filterText_)) {
            filteredRows_.append(row);
        }
    }
    endResetModel();
}

void WorkflowOutlineModel::setFilterText(const QString& filterText)
{
    if (filterText_ == filterText) {
        return;
    }
    filterText_ = filterText;
    refilter();
    Q_EMIT filterTextChanged();
}

void WorkflowOutlineModel::refilter()
{
    beginResetModel();
    filteredRows_.clear();
    for (int row = 0; row < entries_.size(); ++row) {
        const Entry& entry = entries_.at(row);
        if (matchesTerms(QString("%1 %2 %3 %4").arg(entry.title, entry.typeName, entry.category, entry.nodeId), filterText_)) {
            filteredRows_.append(row);
        }
    }
    endResetModel();
}

ProblemModel::ProblemModel(QObject* parent) : QAbstractListModel(parent) {}

int ProblemModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : entries_.size();
}

QVariant ProblemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(index.row());
    switch (role) {
    case MessageRole:
        return entry.message;
    case NodeIdRole:
        return entry.nodeId;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProblemModel::roleNames() const
{
    return {{MessageRole, "message"}, {NodeIdRole, "nodeId"}};
}

void ProblemModel::append(const QString& message, const QString& nodeId)
{
    const int row = entries_.size();
    beginInsertRows({}, row, row);
    entries_.append(Entry{message, nodeId});
    endInsertRows();
}

void ProblemModel::clear()
{
    beginResetModel();
    entries_.clear();
    endResetModel();
}

QuickAccessModel::QuickAccessModel(WorkbenchCommandRegistry* commands,
                                   NodeCatalogModel* nodeCatalog,
                                   WorkflowOutlineModel* workflowOutline,
                                   ProblemModel* problems,
                                   QObject* parent)
    : QAbstractListModel(parent),
      commands_(commands),
      nodeCatalog_(nodeCatalog),
      workflowOutline_(workflowOutline),
      problems_(problems)
{
    auto refreshResults = [this] { refresh(); };
    if (commands_) {
        connect(commands_, &QAbstractItemModel::rowsInserted, this, refreshResults);
        connect(commands_, &QAbstractItemModel::dataChanged, this, refreshResults);
    }
    if (workflowOutline_) {
        connect(workflowOutline_, &QAbstractItemModel::modelReset, this, refreshResults);
    }
    if (problems_) {
        connect(problems_, &QAbstractItemModel::rowsInserted, this, refreshResults);
        connect(problems_, &QAbstractItemModel::modelReset, this, refreshResults);
    }
    refresh();
}

int QuickAccessModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : entries_.size();
}

QVariant QuickAccessModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const Entry& entry = entries_.at(index.row());
    switch (role) {
    case KindRole:
        return kindName(entry.kind);
    case IdRole:
        return entry.id;
    case TitleRole:
        return entry.title;
    case DetailRole:
        return entry.detail;
    default:
        return {};
    }
}

QHash<int, QByteArray> QuickAccessModel::roleNames() const
{
    return {{KindRole, "kind"}, {IdRole, "resultId"}, {TitleRole, "title"}, {DetailRole, "detail"}};
}

void QuickAccessModel::setQuery(const QString& query)
{
    if (query_ == query) {
        return;
    }
    query_ = query;
    refresh();
    Q_EMIT queryChanged();
}

void QuickAccessModel::setRecentWorkflowPaths(const QStringList& paths)
{
    if (recentWorkflowPaths_ == paths) {
        return;
    }
    recentWorkflowPaths_ = paths;
    refresh();
}

QuickAccessModel::Entry QuickAccessModel::entryAt(int row) const
{
    return row >= 0 && row < entries_.size() ? entries_.at(row) : Entry{};
}

void QuickAccessModel::refresh()
{
    beginResetModel();
    entries_.clear();
    const QString normalizedQuery = query_.trimmed();
    const bool emptyQuery = normalizedQuery.isEmpty();
    if (commands_) {
        for (const auto& command : commands_->entries()) {
            if (emptyQuery || matchesTerms(QString("%1 %2 %3").arg(command.title, command.category, command.shortcut), normalizedQuery)) {
                entries_.append(Entry{Kind::Command,
                                      command.id,
                                      command.title,
                                      QString("%1%2").arg(command.category,
                                                         command.shortcut.isEmpty() ? QString() : QString(" · %1").arg(command.shortcut)),
                                      {},
                                      {},
                                      {}});
            }
        }
    }
    if (nodeCatalog_) {
        for (const auto& descriptor : nodeCatalog_->descriptors()) {
            if (!emptyQuery && matchesTerms(QString("%1 %2 %3").arg(descriptor.displayName, descriptor.category, descriptor.typeName), normalizedQuery)) {
                entries_.append(Entry{Kind::Node,
                                      descriptor.typeName,
                                      QString("添加节点：%1").arg(descriptor.displayName),
                                      QString("%1 · %2 · 内置节点").arg(descriptor.category, descriptor.typeName),
                                      descriptor.typeName,
                                      {},
                                      {}});
            }
        }
    }
    if (workflowOutline_) {
        for (const auto& node : workflowOutline_->entries()) {
            if (!emptyQuery && matchesTerms(QString("%1 %2 %3 %4").arg(node.title, node.typeName, node.category, node.nodeId), normalizedQuery)) {
                entries_.append(Entry{Kind::WorkflowNode,
                                      node.nodeId,
                                      QString("定位节点：%1").arg(node.title),
                                      QString("%1 · %2").arg(node.nodeId, node.category),
                                      {},
                                      node.nodeId,
                                      {}});
            }
        }
    }
    if (problems_) {
        for (const auto& problem : problems_->entries()) {
            if (!problem.nodeId.isEmpty() && !emptyQuery &&
                matchesTerms(QString("%1 %2").arg(problem.message, problem.nodeId), normalizedQuery)) {
                entries_.append(Entry{Kind::Problem,
                                      problem.nodeId,
                                      QString("问题：%1").arg(problem.message),
                                      QString("定位到 %1").arg(problem.nodeId),
                                      {},
                                      problem.nodeId,
                                      {}});
            }
        }
    }
    for (const QString& path : std::as_const(recentWorkflowPaths_)) {
        const QString title = QFileInfo(path).fileName();
        if (emptyQuery || matchesTerms(QString("%1 %2").arg(title, path), normalizedQuery)) {
            entries_.append(Entry{Kind::RecentWorkflow,
                                  path,
                                  QString("打开最近工作流：%1").arg(title),
                                  path,
                                  {},
                                  {},
                                  path});
        }
    }
    if (entries_.size() > 80) {
        entries_.resize(80);
    }
    endResetModel();
}

WorkbenchBridge::WorkbenchBridge(WorkbenchCommandRegistry* commands, QuickAccessModel* quickAccess, QObject* parent)
    : QObject(parent), commands_(commands), quickAccess_(quickAccess)
{
}

void WorkbenchBridge::setDocumentTitle(const QString& title)
{
    if (documentTitle_ == title) {
        return;
    }
    documentTitle_ = title;
    Q_EMIT documentTitleChanged();
}

void WorkbenchBridge::setStatusText(const QString& status)
{
    if (statusText_ == status) {
        return;
    }
    statusText_ = status;
    Q_EMIT statusTextChanged();
}

void WorkbenchBridge::setSelectedNodeText(const QString& selectedNode)
{
    if (selectedNodeText_ == selectedNode) {
        return;
    }
    selectedNodeText_ = selectedNode;
    Q_EMIT selectedNodeTextChanged();
}

void WorkbenchBridge::setZoomText(const QString& zoom)
{
    if (zoomText_ == zoom) {
        return;
    }
    zoomText_ = zoom;
    Q_EMIT zoomTextChanged();
}

void WorkbenchBridge::setActiveSidebar(const QString& sidebar)
{
    if (activeSidebar_ == sidebar || sidebar.trimmed().isEmpty()) {
        return;
    }
    activeSidebar_ = sidebar;
    Q_EMIT activeSidebarChanged();
}

void WorkbenchBridge::setPreviewVisible(bool visible)
{
    if (previewVisible_ == visible) {
        return;
    }
    previewVisible_ = visible;
    Q_EMIT previewVisibleChanged();
    Q_EMIT previewVisibilityRequested(visible);
}

void WorkbenchBridge::setPanelVisible(bool visible)
{
    if (panelVisible_ == visible) {
        return;
    }
    panelVisible_ = visible;
    Q_EMIT panelVisibleChanged();
    Q_EMIT panelVisibilityRequested(visible);
}

void WorkbenchBridge::createNode(const QString& typeName)
{
    if (!typeName.trimmed().isEmpty()) {
        Q_EMIT nodeCreationRequested(typeName);
    }
}

void WorkbenchBridge::focusNode(const QString& nodeId)
{
    if (!nodeId.trimmed().isEmpty()) {
        Q_EMIT nodeFocusRequested(nodeId);
    }
}

void WorkbenchBridge::triggerCommand(const QString& id)
{
    if (commands_) {
        commands_->trigger(id);
    }
}

void WorkbenchBridge::showQuickAccess()
{
    Q_EMIT quickAccessRequested();
}

void WorkbenchBridge::activateQuickAccess(int row)
{
    if (!quickAccess_) {
        return;
    }
    const QuickAccessModel::Entry entry = quickAccess_->entryAt(row);
    switch (entry.kind) {
    case QuickAccessModel::Kind::Command:
        triggerCommand(entry.id);
        break;
    case QuickAccessModel::Kind::Node:
        createNode(entry.typeName);
        break;
    case QuickAccessModel::Kind::WorkflowNode:
    case QuickAccessModel::Kind::Problem:
        focusNode(entry.nodeId);
        break;
    case QuickAccessModel::Kind::RecentWorkflow:
        Q_EMIT recentWorkflowRequested(entry.path);
        break;
    }
    Q_EMIT quickAccessFinished();
}

void WorkbenchBridge::startNodeDrag(const QString& typeName, const QString& title, const QString& category)
{
    if (typeName.trimmed().isEmpty()) {
        return;
    }
    auto* drag = new QDrag(QApplication::activeWindow());
    auto* mime = new QMimeData;
    mime->setData("application/x-imagenode-type", typeName.toUtf8());
    mime->setText(typeName);
    drag->setMimeData(mime);

    QPixmap pixmap(220, 44);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(QRect(0, 0, pixmap.width(), pixmap.height()), QColor("#2a2d2e"));
    painter.fillRect(QRect(0, 0, 3, pixmap.height()), QColor("#3794ff"));
    painter.setPen(QColor("#cccccc"));
    QFont titleFont = qApp ? qApp->font() : QFont();
    titleFont.setPointSize(11);
    titleFont.setWeight(QFont::DemiBold);
    painter.setFont(titleFont);
    painter.drawText(QRect(12, 5, 198, 18), Qt::AlignLeft | Qt::AlignVCenter, title.isEmpty() ? typeName : title);
    painter.setPen(QColor("#969696"));
    QFont detailFont = qApp ? qApp->font() : QFont();
    detailFont.setPointSize(9);
    painter.setFont(detailFont);
    painter.drawText(QRect(12, 24, 198, 16), Qt::AlignLeft | Qt::AlignVCenter, category);
    painter.end();
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction);
}

void WorkbenchBridge::saveWorkflowTemplate()
{
    Q_EMIT workflowTemplateSaveRequested();
}

void WorkbenchBridge::applyWorkflowTemplate(const QString& templateId)
{
    if (!templateId.trimmed().isEmpty()) {
        Q_EMIT workflowTemplateApplyRequested(templateId);
    }
}

void WorkbenchBridge::createCheckpoint()
{
    Q_EMIT checkpointCreateRequested();
}

void WorkbenchBridge::restoreCheckpoint(const QString& checkpointId)
{
    if (!checkpointId.trimmed().isEmpty()) {
        Q_EMIT checkpointRestoreRequested(checkpointId);
    }
}

void WorkbenchBridge::branchFromCheckpoint(const QString& checkpointId)
{
    if (!checkpointId.trimmed().isEmpty()) {
        Q_EMIT checkpointBranchRequested(checkpointId);
    }
}

void WorkbenchBridge::requestWindowMove()
{
    Q_EMIT windowMoveRequested();
}

void WorkbenchBridge::requestWindowMinimize()
{
    Q_EMIT windowMinimizeRequested();
}

void WorkbenchBridge::requestWindowMaximizeToggle()
{
    Q_EMIT windowMaximizeToggleRequested();
}

void WorkbenchBridge::requestWindowClose()
{
    Q_EMIT windowCloseRequested();
}
