#include "gui/WorkflowNodeDelegate.h"

#include "core/NodeParameter.h"
#include "gui/AppTheme.h"
#include "nodes/ImageNode.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
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
      parameterChanged_(std::move(parameterChanged))
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
    if (!parameterPanel_ && node_ && !node_->parameterDefinitions().isEmpty()) {
        parameterPanel_ = buildParameterPanel();
        parameterPanel_->setVisible(expanded_);
    }
    return parameterPanel_;
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
    }
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
    panel->setStyleSheet(QString(R"(
        QWidget#nodeInlineParameters {
            background: #252526;
            border: 1px solid #3c3c3c;
            border-radius: 0px;
        }
        QLabel {
            color: #969696;
            font-weight: 600;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            min-height: 24px;
            padding: 2px 6px;
            color: #cccccc;
            background: #3c3c3c;
            border: 1px solid #454545;
            selection-background-color: #04395e;
            selection-color: #ffffff;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #3794ff;
        }
        QCheckBox {
            color: #cccccc;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 14px;
            height: 14px;
            border: 1px solid #6a6a6a;
            background: #3c3c3c;
        }
        QCheckBox::indicator:checked {
            background: #007acc;
            border-color: #3794ff;
        }
        QPushButton {
            min-height: 24px;
            color: #cccccc;
            background: #2d2d2d;
            border: 1px solid #454545;
            padding: 2px 8px;
        }
        QPushButton:hover {
            background: #3c3c3c;
            border-color: #555555;
        }
    )"));

    auto* form = new QFormLayout(panel);
    form->setContentsMargins(AppTheme::px(9, uiScale_), AppTheme::px(7, uiScale_),
                             AppTheme::px(9, uiScale_), AppTheme::px(7, uiScale_));
    form->setHorizontalSpacing(AppTheme::px(8, uiScale_));
    form->setVerticalSpacing(AppTheme::px(6, uiScale_));
    for (const auto& parameter : node_->parameterDefinitions()) {
        auto* label = new QLabel(parameter.displayName);
        label->setMinimumWidth(AppTheme::px(54, uiScale_));
        form->addRow(label, makeParameterEditor(parameter));
    }
    panel->setMinimumWidth(AppTheme::px(196, uiScale_));
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
        editor->addItems(parameter.options);
        editor->setCurrentText(value.toString());
        QObject::connect(editor, &QComboBox::currentTextChanged, editor, [emitChanged](const QString& next) {
            emitChanged(next);
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
        parameter.type == ParameterType::Color) {
        auto* button = new QPushButton("...");
        button->setFixedWidth(AppTheme::px(34, uiScale_));
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, button, [edit, emitChanged, parameter] {
            QString next;
            if (parameter.type == ParameterType::FileOpen) {
                next = QFileDialog::getOpenFileName(edit, "选择图片", {}, "Images (*.png *.jpg *.jpeg *.bmp *.webp);;All files (*)");
            } else if (parameter.type == ParameterType::FileSave) {
                next = QFileDialog::getSaveFileName(edit, "选择导出路径", edit->text(), "PNG (*.png);;JPEG (*.jpg);;All files (*)");
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
