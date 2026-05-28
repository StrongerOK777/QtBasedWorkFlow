#include "app/CommandLineRunner.h"
#include "gui/AppIcon.h"
#include "gui/MainWindow.h"
#include "nodes/NodeFactory.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QQuickStyle>

#include <exception>

namespace {

void configureParser(QCommandLineParser& parser, QCommandLineOption& noGui, QCommandLineOption& workflow)
{
    parser.setApplicationDescription("基于 Qt 的节点式图像处理工作流编辑器");
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
    QCommandLineOption noGui({"n", "no-gui"}, "不启动图形界面，直接执行工作流。");
    QCommandLineOption workflow({"w", "workflow"}, "工作流 JSON 文件路径。", "file");

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
    try {
        return app.exec();
    } catch (const std::exception& e) {
        qCritical("程序因未处理的异常退出：%s", e.what());
        return 1;
    } catch (...) {
        qCritical("程序因未处理的未知异常退出");
        return 1;
    }
}
