#pragma once

#include <QtNodes/AbstractNodePainter>
#include <QtNodes/DefaultNodePainter>

class WorkflowNodePainter final : public QtNodes::AbstractNodePainter {
public:
    void paint(QPainter* painter, QtNodes::NodeGraphicsObject& node) const override;

private:
    QtNodes::DefaultNodePainter defaultPainter_;
};
