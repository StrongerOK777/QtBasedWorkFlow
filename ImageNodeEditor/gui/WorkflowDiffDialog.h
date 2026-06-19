#pragma once

#include "core/NodeLabel.h"
#include "core/NodeParameter.h"
#include "gui/AppTheme.h"
#include "gui/PreviewWidgets.h"
#include "nodes/ImageNode.h"
#include "workflow/WorkflowGraph.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QPair>
#include <QPixmap>
#include <QSet>
#include <QVBoxLayout>
#include <QVector>

#include <cmath>
#include <functional>

// 工作流对比对话框：两侧各选一个版本（保存点快照或当前画布），上半区并排显示
// 各自的最终输出缩略图（点击看大图），下半区按 新增/删除/参数修改/连线变化 分组
// 列出结构化差异。与 PreviewWidgets.h 同模式的 header-only 内部控件，无 Q_OBJECT。
namespace WorkflowDiffInternal {

// 输出缩略图块：棋盘格底 + 等比缩放；有图时点击弹出大图查看窗口。
class DiffThumbLabel final : public QLabel {
public:
    explicit DiffThumbLabel(double uiScale, QWidget* parent = nullptr)
        : QLabel(parent), uiScale_(uiScale)
    {
        setAlignment(Qt::AlignCenter);
        setMinimumSize(AppTheme::px(280, uiScale_), AppTheme::px(170, uiScale_));
        setFrameShape(QFrame::NoFrame);
    }

    void setImage(const QImage& image)
    {
        image_ = image;
        if (image_.isNull()) {
            setCursor(Qt::ArrowCursor);
            setPixmap({});
            setText("无缩略图（保存时未执行）");
            return;
        }
        setCursor(Qt::PointingHandCursor);
        setToolTip("点击查看大图");
        const QImage scaled = image_.scaled(minimumWidth(), minimumHeight(),
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap pixmap(scaled.size());
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            PreviewInternal::drawCheckerboard(painter, QRectF(QPointF(0, 0), QSizeF(scaled.size())));
            painter.drawImage(0, 0, scaled);
        }
        setPixmap(pixmap);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && !image_.isNull()) {
            auto* popup = new ImagePopupWindow(image_, this);
            popup->setWindowTitle("输出结果");
            popup->show();
            event->accept();
            return;
        }
        QLabel::mousePressEvent(event);
    }

private:
    double uiScale_ = 1.0;
    QImage image_;
};

}

class WorkflowDiffDialog final : public QDialog {
public:
    struct Entry {
        QString label;
        WorkflowGraph graph;
        QImage thumbnail;  // 可空：保存时未执行过
    };

    WorkflowDiffDialog(QVector<Entry> entries, double uiScale, QWidget* parent = nullptr)
        : QDialog(parent), entries_(std::move(entries)), uiScale_(uiScale)
    {
        setWindowTitle("对比工作流版本");
        resize(AppTheme::px(720, uiScale_), AppTheme::px(640, uiScale_));

        auto* root = new QVBoxLayout(this);
        root->setSpacing(AppTheme::px(10, uiScale_));

        auto* sides = new QHBoxLayout;
        sides->setSpacing(AppTheme::px(12, uiScale_));
        auto makeSide = [this](QComboBox*& combo, WorkflowDiffInternal::DiffThumbLabel*& thumb) {
            auto* column = new QVBoxLayout;
            column->setSpacing(AppTheme::px(6, uiScale_));
            combo = new QComboBox;
            for (const Entry& entry : entries_) {
                combo->addItem(entry.label);
            }
            column->addWidget(combo);
            thumb = new WorkflowDiffInternal::DiffThumbLabel(uiScale_);
            column->addWidget(thumb, 1);
            return column;
        };
        sides->addLayout(makeSide(leftCombo_, leftThumb_), 1);
        sides->addLayout(makeSide(rightCombo_, rightThumb_), 1);
        root->addLayout(sides);

        summary_ = new QLabel;
        root->addWidget(summary_);

        list_ = new QListWidget;
        list_->setSelectionMode(QAbstractItemView::NoSelection);
        list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_->setWordWrap(true);
        root->addWidget(list_, 1);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        root->addWidget(buttons);

        // 默认：左 = 最新保存点（若有），右 = 第一项（调用方把「当前画布」放在首位）。
        leftCombo_->setCurrentIndex(entries_.size() > 1 ? 1 : 0);
        rightCombo_->setCurrentIndex(0);
        connect(leftCombo_, &QComboBox::currentIndexChanged, this, [this](int) { recompute(); });
        connect(rightCombo_, &QComboBox::currentIndexChanged, this, [this](int) { recompute(); });
        recompute();
    }

private:
    struct NodeSnapshot {
        QString typeName;
        QString label;
        QJsonObject params;
        QPointF position;
        QSharedPointer<ImageNode> node;
    };

    static QMap<QString, NodeSnapshot> collectNodes(const WorkflowGraph& graph)
    {
        QMap<QString, NodeSnapshot> snapshots;
        for (const WorkflowNodeRecord& record : graph.nodes()) {
            if (!record.node) {
                continue;
            }
            NodeSnapshot snapshot;
            snapshot.typeName = record.node->typeName();
            snapshot.label = formatNodeLabel(record.node->displayName(), record.id);
            snapshot.params = record.node->saveParams();
            snapshot.position = record.position;
            snapshot.node = record.node;
            snapshots.insert(record.id, snapshot);
        }
        return snapshots;
    }

    static QString jsonValueText(const QJsonValue& value)
    {
        if (value.isUndefined() || value.isNull()) {
            return QStringLiteral("（空）");
        }
        if (value.isBool()) {
            return value.toBool() ? QStringLiteral("开") : QStringLiteral("关");
        }
        const QString text = value.toVariant().toString();
        return text.isEmpty() ? QStringLiteral("（空）") : text;
    }

    static QString parameterDisplayName(const QSharedPointer<ImageNode>& node, const QString& name)
    {
        if (node) {
            for (const NodeParameter& parameter : node->parameterDefinitions()) {
                if (parameter.name == name) {
                    return parameter.displayName.isEmpty() ? name : parameter.displayName;
                }
            }
        }
        return name;
    }

    void addSection(const QString& title)
    {
        auto* item = new QListWidgetItem(title);
        item->setFlags(Qt::NoItemFlags);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
        item->setForeground(AppTheme::palette().textMuted);
        list_->addItem(item);
    }

    void addLine(const QString& text, const QColor& color)
    {
        auto* item = new QListWidgetItem("  " + text);
        item->setFlags(Qt::ItemIsEnabled);
        item->setForeground(color);
        list_->addItem(item);
    }

    void recompute()
    {
        const int leftIndex = leftCombo_->currentIndex();
        const int rightIndex = rightCombo_->currentIndex();
        if (leftIndex < 0 || rightIndex < 0 || leftIndex >= entries_.size() || rightIndex >= entries_.size()) {
            return;
        }
        const Entry& left = entries_.at(leftIndex);
        const Entry& right = entries_.at(rightIndex);
        leftThumb_->setImage(left.thumbnail);
        rightThumb_->setImage(right.thumbnail);

        list_->clear();
        if (leftIndex == rightIndex) {
            summary_->setText("两侧选择了同一个版本。");
            return;
        }

        const AppTheme::Palette palette = AppTheme::palette();
        const QColor addedColor = palette.accent;
        const QColor removedColor = palette.danger;
        const QColor changedColor = palette.textPrimary;

        const QMap<QString, NodeSnapshot> before = collectNodes(left.graph);
        const QMap<QString, NodeSnapshot> after = collectNodes(right.graph);
        int differences = 0;

        // 节点：以左侧为旧、右侧为新。同 id 不同类型按「删除 + 新增」呈现。
        QStringList addedNodes;
        QStringList removedNodes;
        QVector<QPair<QString, QString>> paramLines;  // <节点标签, 描述>
        int movedCount = 0;
        for (auto it = after.cbegin(); it != after.cend(); ++it) {
            const auto old = before.constFind(it.key());
            if (old == before.cend() || old.value().typeName != it.value().typeName) {
                addedNodes.append(it.value().label);
            }
        }
        for (auto it = before.cbegin(); it != before.cend(); ++it) {
            const auto now = after.constFind(it.key());
            if (now == after.cend() || now.value().typeName != it.value().typeName) {
                removedNodes.append(it.value().label);
                continue;
            }
            // 参数：取两侧键的并集逐个比较（排序保证清单顺序稳定）。
            // 注意 QJsonObject 没有 cbegin/cend，用 keys() 取键列表。
            QSet<QString> keySet;
            const QStringList beforeKeys = it.value().params.keys();
            for (const QString& key : beforeKeys) {
                keySet.insert(key);
            }
            const QStringList afterKeys = now.value().params.keys();
            for (const QString& key : afterKeys) {
                keySet.insert(key);
            }
            QStringList keys = keySet.values();
            keys.sort();
            for (const QString& key : keys) {
                const QJsonValue oldValue = it.value().params.value(key);
                const QJsonValue newValue = now.value().params.value(key);
                if (oldValue != newValue) {
                    paramLines.append({it.value().label,
                                       QString("%1：%2 → %3")
                                           .arg(parameterDisplayName(now.value().node, key),
                                                jsonValueText(oldValue), jsonValueText(newValue))});
                }
            }
            const QPointF delta = it.value().position - now.value().position;
            if (std::abs(delta.x()) > 0.5 || std::abs(delta.y()) > 0.5) {
                ++movedCount;
            }
        }

        // 连线：按四元组比较；描述用所在侧的节点标签。
        auto edgeKey = [](const Edge& edge) {
            return edge.fromNode + "|" + edge.fromPort + "|" + edge.toNode + "|" + edge.toPort;
        };
        auto edgeText = [](const Edge& edge, const QMap<QString, NodeSnapshot>& nodes) {
            const QString from = nodes.contains(edge.fromNode) ? nodes.value(edge.fromNode).label : edge.fromNode;
            const QString to = nodes.contains(edge.toNode) ? nodes.value(edge.toNode).label : edge.toNode;
            return QString("%1.%2 → %3.%4").arg(from, edge.fromPort, to, edge.toPort);
        };
        QSet<QString> beforeEdges;
        for (const Edge& edge : left.graph.edges()) {
            beforeEdges.insert(edgeKey(edge));
        }
        QSet<QString> afterEdges;
        for (const Edge& edge : right.graph.edges()) {
            afterEdges.insert(edgeKey(edge));
        }
        QStringList addedEdges;
        for (const Edge& edge : right.graph.edges()) {
            if (!beforeEdges.contains(edgeKey(edge))) {
                addedEdges.append(edgeText(edge, after));
            }
        }
        QStringList removedEdges;
        for (const Edge& edge : left.graph.edges()) {
            if (!afterEdges.contains(edgeKey(edge))) {
                removedEdges.append(edgeText(edge, before));
            }
        }

        if (!addedNodes.isEmpty()) {
            addSection(QString("新增节点（%1）").arg(addedNodes.size()));
            for (const QString& line : addedNodes) {
                addLine(line, addedColor);
            }
            differences += addedNodes.size();
        }
        if (!removedNodes.isEmpty()) {
            addSection(QString("删除节点（%1）").arg(removedNodes.size()));
            for (const QString& line : removedNodes) {
                addLine(line, removedColor);
            }
            differences += removedNodes.size();
        }
        if (!paramLines.isEmpty()) {
            addSection(QString("参数修改（%1）").arg(paramLines.size()));
            for (const auto& line : paramLines) {
                addLine(QString("%1：%2").arg(line.first, line.second), changedColor);
            }
            differences += paramLines.size();
        }
        if (!addedEdges.isEmpty()) {
            addSection(QString("新增连线（%1）").arg(addedEdges.size()));
            for (const QString& line : addedEdges) {
                addLine(line, addedColor);
            }
            differences += addedEdges.size();
        }
        if (!removedEdges.isEmpty()) {
            addSection(QString("删除连线（%1）").arg(removedEdges.size()));
            for (const QString& line : removedEdges) {
                addLine(line, removedColor);
            }
            differences += removedEdges.size();
        }
        if (movedCount > 0) {
            addSection("布局");
            addLine(QString("%1 个节点位置变化").arg(movedCount), palette.textSecondary);
        }

        summary_->setText(differences == 0 && movedCount == 0
                              ? QString("「%1」与「%2」内容一致。").arg(left.label, right.label)
                              : QString("「%1」 → 「%2」：共 %3 处结构差异%4")
                                    .arg(left.label, right.label)
                                    .arg(differences)
                                    .arg(movedCount > 0 ? QString("，另有 %1 个节点仅移动位置").arg(movedCount)
                                                        : QString()));
    }

    QVector<Entry> entries_;
    double uiScale_ = 1.0;
    QComboBox* leftCombo_ = nullptr;
    QComboBox* rightCombo_ = nullptr;
    WorkflowDiffInternal::DiffThumbLabel* leftThumb_ = nullptr;
    WorkflowDiffInternal::DiffThumbLabel* rightThumb_ = nullptr;
    QLabel* summary_ = nullptr;
    QListWidget* list_ = nullptr;
};
