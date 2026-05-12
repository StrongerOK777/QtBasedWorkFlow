#include "gui/WorkflowCommands.h"

#include "gui/MainWindow.h"
#include "nodes/ImageNode.h"
#include "nodes/NodeFactory.h"

#include <QUndoCommand>

namespace {

class SnapshotCommand final : public QUndoCommand {
public:
    SnapshotCommand(MainWindow* window,
                    QString text,
                    const WorkflowGraph& before,
                    const WorkflowGraph& after,
                    QString selectedBefore,
                    QString selectedAfter,
                    QString mergeKey)
        : QUndoCommand(std::move(text)),
          window_(window),
          before_(WorkflowCommands::cloneGraph(before)),
          after_(WorkflowCommands::cloneGraph(after)),
          selectedBefore_(std::move(selectedBefore)),
          selectedAfter_(std::move(selectedAfter)),
          mergeKey_(std::move(mergeKey))
    {
    }

    int id() const override
    {
        return mergeKey_.isEmpty() ? -1 : int(qHash(mergeKey_) & 0x7fffffff);
    }

    bool mergeWith(const QUndoCommand* other) override
    {
        const auto* next = dynamic_cast<const SnapshotCommand*>(other);
        if (!next || mergeKey_.isEmpty() || mergeKey_ != next->mergeKey_) {
            return false;
        }
        after_ = WorkflowCommands::cloneGraph(next->after_);
        selectedAfter_ = next->selectedAfter_;
        return true;
    }

    void undo() override
    {
        if (window_) {
            window_->restoreGraphFromUndo(before_, selectedBefore_);
        }
    }

    void redo() override
    {
        if (firstRedo_) {
            firstRedo_ = false;
            return;
        }
        if (window_) {
            window_->restoreGraphFromUndo(after_, selectedAfter_);
        }
    }

private:
    MainWindow* window_ = nullptr;
    WorkflowGraph before_;
    WorkflowGraph after_;
    QString selectedBefore_;
    QString selectedAfter_;
    QString mergeKey_;
    bool firstRedo_ = true;
};

}

namespace WorkflowCommands {

WorkflowGraph cloneGraph(const WorkflowGraph& graph)
{
    WorkflowGraph copy;
    NodeFactory::instance().registerBuiltins();
    for (const auto& record : graph.nodes()) {
        auto created = NodeFactory::instance().create(record.node->typeName());
        if (created.isFail()) {
            continue;
        }
        auto loaded = created.value()->loadParams(record.node->saveParams());
        if (loaded.isFail()) {
            continue;
        }
        copy.addNodeWithId(record.id, created.value(), record.position);
    }
    for (const auto& edge : graph.edges()) {
        copy.addEdge(edge);
    }
    return copy;
}

QUndoCommand* makeSnapshotCommand(MainWindow* window,
                                  const QString& text,
                                  const WorkflowGraph& before,
                                  const WorkflowGraph& after,
                                  const QString& selectedBefore,
                                  const QString& selectedAfter,
                                  const QString& mergeKey)
{
    return new SnapshotCommand(window, text, before, after, selectedBefore, selectedAfter, mergeKey);
}

}
