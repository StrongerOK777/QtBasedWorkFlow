#include "gui/WorkflowNodeDelegate.h"

#include "core/NodeParameter.h"
#include "gui/AppTheme.h"
#include "gui/PreviewWidgets.h"
#include "nodes/ImageNode.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace {

class WorkflowPortData final : public QtNodes::NodeData {
public:
    explicit WorkflowPortData(QtNodes::NodeDataType type) : type_(std::move(type)) {}

    QtNodes::NodeDataType type() const override { return type_; }

private:
    QtNodes::NodeDataType type_;
};

QVector<PortInfo> ports(const QSharedPointer<ImageNode>& node, QtNodes::PortType portType)
{
    if (!node) {
        return {};
    }
    return portType == QtNodes::PortType::In ? node->inputPorts() : node->outputPorts();
}

// 把存储用的英文选项值映射为界面显示的中文，未知值回退原值。
QString choiceOptionLabel(const QString& value)
{
    static const QHash<QString, QString> kLabels = {
        {"horizontal", "横向"},
        {"vertical", "纵向"},
        {"grid", "网格"},
        {"normal", "正常"},
        {"rotate90", "旋转 90°"},
        {"rotate180", "旋转 180°"},
        {"rotate270", "旋转 270°"},
        {"flipHorizontal", "水平翻转"},
        {"flipVertical", "垂直翻转"},
    };
    const auto it = kLabels.constFind(value);
    return it != kLabels.constEnd() ? it.value() : value;
}

QtNodes::NodeDataType workflowDataType(PortType type)
{
    switch (type) {
    case PortType::ImageRGBA:
    case PortType::ImageGray:
        return {"image", "图片"};
    case PortType::Number:
        return {"number", "数值"};
    case PortType::Text:
        return {"text", "文本"};
    case PortType::Mask:
        return {"mask", "蒙版"};
    case PortType::ImageList:
        return {"image-list", "图片列表"};
    }
    return {"unknown", "未知"};
}

QtNodes::NodeProcessingStatus nodeProcessingStatus(NodeExecutionState state)
{
    switch (state) {
    case NodeExecutionState::Running:
        return QtNodes::NodeProcessingStatus::Processing;
    case NodeExecutionState::Succeeded:
    case NodeExecutionState::CacheHit:
        return QtNodes::NodeProcessingStatus::Updated;
    case NodeExecutionState::Failed:
        return QtNodes::NodeProcessingStatus::Failed;
    case NodeExecutionState::NotExecuted:
        return QtNodes::NodeProcessingStatus::NoStatus;
    }
    return QtNodes::NodeProcessingStatus::NoStatus;
}

}

WorkflowNodeDelegate::WorkflowNodeDelegate(QString workflowNodeId,
                                           QString category,
                                           QSharedPointer<ImageNode> node,
                                           double uiScale,
                                           ParameterChanged parameterChanged)
    : workflowNodeId_(std::move(workflowNodeId)),
      category_(std::move(category)),
      node_(std::move(node)),
      uiScale_(uiScale),
      parameterChanged_(std::move(parameterChanged)),
      thumbnailsEnabled_(QSettings().value("mainWindow/nodeThumbnails", true).toBool())
{
}

QString WorkflowNodeDelegate::name() const
{
    return node_ ? node_->typeName() : QStringLiteral("Unknown");
}

QString WorkflowNodeDelegate::caption() const
{
    return node_ ? node_->displayName() : QStringLiteral("未知节点");
}

QString WorkflowNodeDelegate::portCaption(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    const auto currentPorts = ports(node_, portType);
    return index < unsigned(currentPorts.size()) ? currentPorts.at(int(index)).displayName : QString();
}

unsigned int WorkflowNodeDelegate::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::None) {
        return 0;
    }
    return unsigned(ports(node_, portType).size());
}

QtNodes::NodeDataType WorkflowNodeDelegate::dataType(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    const auto currentPorts = ports(node_, portType);
    if (index >= unsigned(currentPorts.size())) {
        return {"unknown", "未知"};
    }
    return workflowDataType(currentPorts.at(int(index)).type);
}

QtNodes::ConnectionPolicy WorkflowNodeDelegate::portConnectionPolicy(QtNodes::PortType portType,
                                                                     QtNodes::PortIndex index) const
{
    const auto currentPorts = ports(node_, portType);
    if (index >= unsigned(currentPorts.size())) {
        return QtNodes::ConnectionPolicy::One;
    }
    const PortInfo port = currentPorts.at(int(index));
    return port.allowMultipleConnections ? QtNodes::ConnectionPolicy::Many : QtNodes::ConnectionPolicy::One;
}

std::shared_ptr<QtNodes::NodeData> WorkflowNodeDelegate::outData(QtNodes::PortIndex port)
{
    return std::make_shared<WorkflowPortData>(dataType(QtNodes::PortType::Out, port));
}

void WorkflowNodeDelegate::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex port)
{
    Q_UNUSED(nodeData);
    Q_UNUSED(port);
}

QWidget* WorkflowNodeDelegate::embeddedWidget()
{
    // 所有节点都返回一个容器：缩略图（执行后常显）在上，参数面板（选中展开时可见）在下。
    // QtNodes 只在建图元时取一次 embeddedWidget，因此容器必须从一开始就存在。
    if (!container_) {
        container_ = new QWidget;
        auto* layout = new QVBoxLayout(container_);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(AppTheme::px(4, uiScale_));

        thumbLabel_ = new QLabel;
        thumbLabel_->setFixedWidth(AppTheme::px(184, uiScale_));
        thumbLabel_->setAlignment(Qt::AlignCenter);
        thumbLabel_->setVisible(false);
        layout->addWidget(thumbLabel_);

        if (node_ && !node_->parameterDefinitions().isEmpty()) {
            parameterPanel_ = buildParameterPanel();
            parameterPanel_->setVisible(expanded_);
            layout->addWidget(parameterPanel_);
        }
        container_->adjustSize();
    }
    return container_;
}

void WorkflowNodeDelegate::setExpanded(bool expanded)
{
    if (expanded_ == expanded) {
        return;
    }
    expanded_ = expanded;
    if (parameterPanel_) {
        parameterPanel_->setVisible(expanded_);
        parameterPanel_->adjustSize();
        if (container_) {
            container_->adjustSize();
        }
    }
    Q_EMIT requestNodeUpdate();
}

void WorkflowNodeDelegate::setOutputThumbnail(const QImage& image)
{
    if (!thumbLabel_) {
        embeddedWidget();
    }
    if (!thumbLabel_) {
        return;
    }
    const bool wasVisible = thumbLabel_->isVisible();
    if (!thumbnailsEnabled_ || image.isNull()) {
        if (!wasVisible) {
            return;
        }
        thumbLabel_->clear();
        thumbLabel_->setVisible(false);
        thumbLabel_->setFixedHeight(0);
    } else {
        const int width = AppTheme::px(184, uiScale_);
        const int maxHeight = AppTheme::px(118, uiScale_);
        const QImage scaled = image.scaled(width, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap pixmap(scaled.size());
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            PreviewInternal::drawCheckerboard(painter, QRectF(QPointF(0, 0), QSizeF(scaled.size())));
            painter.drawImage(0, 0, scaled);
        }
        thumbLabel_->setFixedHeight(scaled.height());
        thumbLabel_->setPixmap(pixmap);
        thumbLabel_->setVisible(true);
    }
    if (container_) {
        container_->adjustSize();
    }
    Q_EMIT embeddedWidgetSizeUpdated();
    Q_EMIT requestNodeUpdate();
}

void WorkflowNodeDelegate::setExecutionState(NodeExecutionState state)
{
    executionState_ = state;
    setNodeProcessingStatus(nodeProcessingStatus(state));
    if (state == NodeExecutionState::CacheHit) {
        setProgressValue("缓存");
    } else if (elapsedMs_ >= 0) {
        setProgressValue(QString("%1 ms").arg(elapsedMs_));
    } else {
        setProgressValue({});
    }
    Q_EMIT requestNodeUpdate();
}

void WorkflowNodeDelegate::setElapsedMs(qint64 elapsedMs)
{
    elapsedMs_ = elapsedMs;
    if (executionState_ != NodeExecutionState::CacheHit) {
        setProgressValue(elapsedMs_ >= 0 ? QString("%1 ms").arg(elapsedMs_) : QString());
    }
    Q_EMIT requestNodeUpdate();
}

void WorkflowNodeDelegate::setAnimationPhase(int phase)
{
    animationPhase_ = phase;
    if (executionState_ == NodeExecutionState::Running) {
        Q_EMIT requestNodeUpdate();
    }
}

QWidget* WorkflowNodeDelegate::buildParameterPanel()
{
    auto* panel = new QWidget;
    panel->setObjectName("nodeInlineParameters");
    // 内联参数面板配色跟随当前主题 palette（深色 / 浅色一致）。
    const AppTheme::Palette p = AppTheme::palette();
    QString css = QString(R"(
        QWidget#nodeInlineParameters {
            background: @elevated@;
            border: 1px solid @border@;
            border-radius: 10px;
        }
        QLabel {
            color: @textSecondary@;
            font-weight: 500;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            min-height: 22px;
            padding: 1px 6px;
            color: @textPrimary@;
            background: @input@;
            border: 1px solid @hairline@;
            border-radius: 6px;
            selection-background-color: @selection@;
            selection-color: @selectionText@;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: @accent@;
        }
        QCheckBox {
            color: @textPrimary@;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 14px;
            height: 14px;
            border: 1px solid @border@;
            border-radius: 4px;
            background: @input@;
        }
        QCheckBox::indicator:checked {
            background: @accent@;
            border-color: @accent@;
        }
        QPushButton {
            min-height: 22px;
            color: @textPrimary@;
            background: @input@;
            border: 1px solid @hairline@;
            border-radius: 6px;
            padding: 1px 8px;
        }
        QPushButton:hover {
            background: @elevatedHover@;
            border-color: @border@;
        }
    )");
    css.replace("@elevated@", p.elevated.name())
        .replace("@elevatedHover@", p.elevatedHover.name())
        .replace("@input@", p.input.name())
        .replace("@hairline@", p.hairline.name())
        .replace("@border@", p.border.name())
        .replace("@textPrimary@", p.textPrimary.name())
        .replace("@textSecondary@", p.textSecondary.name())
        .replace("@selectionText@", p.selectionText.name())
        .replace("@selection@", p.selection.name())
        .replace("@accent@", p.accent.name());
    panel->setStyleSheet(css);

    auto* form = new QFormLayout(panel);
    form->setContentsMargins(AppTheme::px(6, uiScale_), AppTheme::px(4, uiScale_),
                             AppTheme::px(6, uiScale_), AppTheme::px(4, uiScale_));
    form->setHorizontalSpacing(AppTheme::px(6, uiScale_));
    form->setVerticalSpacing(AppTheme::px(3, uiScale_));
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    for (const auto& parameter : node_->parameterDefinitions()) {
        auto* label = new QLabel(parameter.displayName);
        label->setMinimumWidth(AppTheme::px(40, uiScale_));
        form->addRow(label, makeParameterEditor(parameter));
    }
    // 固定宽度让所有节点宽度统一（高度随参数行数略变，符合「大小基本一致」）。
    panel->setFixedWidth(AppTheme::px(184, uiScale_));
    panel->setMinimumHeight(AppTheme::px(34, uiScale_));
    panel->adjustSize();
    return panel;
}

QWidget* WorkflowNodeDelegate::makeParameterEditor(const NodeParameter& parameter)
{
    const QVariant value = node_->parameterValue(parameter.name);
    auto emitChanged = [this, name = parameter.name](const QVariant& next) {
        if (parameterChanged_ && !workflowNodeId_.isEmpty()) {
            parameterChanged_(workflowNodeId_, name, next);
        }
    };

    if (parameter.type == ParameterType::Integer) {
        auto* editor = new QSpinBox;
        editor->setRange(int(parameter.min), int(parameter.max));
        editor->setValue(value.toInt());
        QObject::connect(editor, &QSpinBox::valueChanged, editor, [emitChanged](int next) { emitChanged(next); });
        return editor;
    }
    if (parameter.type == ParameterType::Double) {
        auto* editor = new QDoubleSpinBox;
        editor->setRange(parameter.min, parameter.max);
        editor->setSingleStep(0.05);
        editor->setValue(value.toDouble());
        QObject::connect(editor, &QDoubleSpinBox::valueChanged, editor, [emitChanged](double next) { emitChanged(next); });
        return editor;
    }
    if (parameter.type == ParameterType::Boolean) {
        auto* editor = new QCheckBox;
        editor->setChecked(value.toBool());
        QObject::connect(editor, &QCheckBox::toggled, editor, [emitChanged](bool next) { emitChanged(next); });
        return editor;
    }
    if (parameter.type == ParameterType::Choice) {
        auto* editor = new QComboBox;
        for (const QString& option : parameter.options) {
            editor->addItem(choiceOptionLabel(option), option);
        }
        int index = editor->findData(value.toString());
        if (index < 0) {
            index = 0;
        }
        editor->setCurrentIndex(index);
        QObject::connect(editor, &QComboBox::currentIndexChanged, editor, [editor, emitChanged](int) {
            emitChanged(editor->currentData().toString());
        });
        return editor;
    }

    auto* container = new QWidget;
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(AppTheme::px(6, uiScale_));
    auto* edit = new QLineEdit;
    edit->setText(value.toString());
    layout->addWidget(edit);
    if (parameter.type == ParameterType::FileOpen || parameter.type == ParameterType::FileSave ||
        parameter.type == ParameterType::Directory || parameter.type == ParameterType::Color) {
        auto* button = new QPushButton("...");
        button->setFixedWidth(AppTheme::px(26, uiScale_));
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, button, [edit, emitChanged, parameter] {
            QString next;
            if (parameter.type == ParameterType::FileOpen) {
                next = QFileDialog::getOpenFileName(edit, "选择图片", {}, "图片 (*.png *.jpg *.jpeg *.bmp *.webp);;所有文件 (*)");
            } else if (parameter.type == ParameterType::FileSave) {
                next = QFileDialog::getSaveFileName(edit, "选择导出路径", edit->text(), "PNG 图片 (*.png);;JPEG 图片 (*.jpg);;所有文件 (*)");
            } else if (parameter.type == ParameterType::Directory) {
                next = QFileDialog::getExistingDirectory(edit, "选择目录", edit->text());
            } else {
                const QColor color = QColorDialog::getColor(QColor(edit->text()), edit);
                if (color.isValid()) {
                    next = color.name();
                }
            }
            if (!next.isEmpty()) {
                edit->setText(next);
                emitChanged(next);
            }
        });
    }
    QObject::connect(edit, &QLineEdit::editingFinished, edit, [edit, emitChanged] { emitChanged(edit->text()); });
    return container;
}
