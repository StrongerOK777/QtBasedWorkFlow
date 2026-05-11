#include "app/CommandLineRunner.h"

#include "nodes/NodeFactory.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowSerializer.h"

#include <QTextStream>

int CommandLineRunner::run(const QString& workflowPath) const
{
    QTextStream err(stderr);
    QTextStream out(stdout);
    NodeFactory::instance().registerBuiltins();
    WorkflowSerializer serializer;
    auto graph = serializer.loadFile(workflowPath);
    if (graph.isFail()) {
        err << graph.error() << Qt::endl;
        return 2;
    }
    ExecutionEngine engine;
    auto result = engine.execute(graph.value());
    if (result.isFail()) {
        err << result.error() << Qt::endl;
        return 3;
    }
    for (const QString& line : result.value().log) {
        out << line << Qt::endl;
    }
    return 0;
}
