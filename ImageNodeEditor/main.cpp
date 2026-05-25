#include "app/CommandLineRunner.h"
#include "gui/AppIcon.h"
#include "gui/MainWindow.h"
#include "nodes/NodeFactory.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QQuickStyle>

namespace {

void configureParser(QCommandLineParser& parser, QCommandLineOption& noGui, QCommandLineOption& workflow)
{
    parser.setApplicationDescription("Qt node-based image processing workflow editor");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(noGui);
    parser.addOption(workflow);
}

bool hasCliOnlyArgument(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--no-gui" || arg == "-n" || arg == "--help" || arg == "-h" || arg == "--version" || arg == "-v") {
            return true;
        }
    }
    return false;
}

}

int main(int argc, char* argv[])
{
    QCommandLineOption noGui({"n", "no-gui"}, "Run workflow without GUI.");
    QCommandLineOption workflow({"w", "workflow"}, "Workflow JSON path.", "file");

    if (hasCliOnlyArgument(argc, argv)) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName("ImageNodeEditor");
        QCoreApplication::setOrganizationName("ALPDHomework");
        QCoreApplication::setApplicationVersion("0.1.0");
        NodeFactory::instance().registerBuiltins();
        QCommandLineParser parser;
        configureParser(parser, noGui, workflow);
        parser.process(app);
        if (!parser.isSet(noGui)) {
            return 0;
        }
        const QString path = parser.value(workflow);
        if (path.isEmpty()) {
            fprintf(stderr, "--no-gui requires --workflow <file>\n");
            return 2;
        }
        return CommandLineRunner().run(path);
    }

    QQuickStyle::setStyle("Basic");
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("ImageNodeEditor");
    QCoreApplication::setOrganizationName("ALPDHomework");
    QCoreApplication::setApplicationVersion("0.1.0");
    app.setWindowIcon(AppIcon::makeAppIcon());
    NodeFactory::instance().registerBuiltins();

    QCommandLineParser parser;
    configureParser(parser, noGui, workflow);
    parser.process(app);

    MainWindow window;
    window.show();
    return app.exec();
}
