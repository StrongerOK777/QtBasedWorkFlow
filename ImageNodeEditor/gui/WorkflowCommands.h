#pragma once

#include "workflow/WorkflowGraph.h"

#include <QString>

class MainWindow;
class QUndoCommand;

namespace WorkflowCommands {

WorkflowGraph cloneGraph(const WorkflowGraph& graph);

QUndoCommand* makeSnapshotCommand(MainWindow* window,
                                  const QString& text,
                                  const WorkflowGraph& before,
                                  const WorkflowGraph& after,
                                  const QString& selectedBefore,
                                  const QString& selectedAfter,
                                  const QString& mergeKey = {});

}
