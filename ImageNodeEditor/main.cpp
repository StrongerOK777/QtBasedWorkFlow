#include "app/CommandLineApp.h"
#include "app/CommandLineRunner.h"
#include "gui/AppIcon.h"
#include "gui/AppTheme.h"
#include "gui/MainWindow.h"
#include "gui/NativeWindowChrome.h"
#include "nodes/NodeFactory.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickStyle>
#include <QSettings>
#include <QStringList>
#include <QSurfaceFormat>

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

QStringList collectArgs(int argc, char* argv[])
{
    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args << QString::fromLocal8Bit(argv[i]);
    }
    return args;
}

bool hasLegacyNoGui(const QStringList& args)
{
    return args.contains("--no-gui") || args.contains("-n");
}

bool startsWithCliEntry(const QStringList& args)
{
    if (args.isEmpty()) {
        return false;
    }
    const QString& first = args.first();
    return CommandLineApp::isSubcommand(first) || first == "--help" || first == "-h"
        || first == "--version" || first == "-v";
}

}

int main(int argc, char* argv[])
{
    QCommandLineOption noGui({"n", "no-gui"}, "不启动图形界面，直接执行工作流。");
    QCommandLineOption workflow({"w", "workflow"}, "工作流 JSON 文件路径。", "file");

    const QStringList cliArgs = collectArgs(argc, argv);
    const bool cliEntry = startsWithCliEntry(cliArgs);
    if (cliEntry || hasLegacyNoGui(cliArgs)) {
        // 命令行模式不显示窗口，但文字叠加等节点需要字体/QPainter 渲染（仅 QCoreApplication
        // 无法渲染文字）。改用 QGuiApplication，并默认 offscreen 平台：无显示也能渲染、不弹窗。
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
            qputenv("QT_QPA_PLATFORM", "offscreen");
        }
        QGuiApplication app(argc, argv);
        QCoreApplication::setApplicationName("ImageNodeEditor");
        QCoreApplication::setOrganizationName("ALPDHomework");
        QCoreApplication::setApplicationVersion("0.1.0");
        NodeFactory::instance().registerBuiltins();
        // 新版 picdeal 子命令（pipe/build/run/nodes/log/restore...）走 CommandLineApp。
        if (cliEntry) {
            return CommandLineApp::run(cliArgs);
        }
        // 向后兼容旧的 --no-gui --workflow 入口。
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
    // 让 QQuickWidget 的透明清屏真正带 alpha 通道，否则命令面板等圆角浮层的四角会清成黑色（黑尖角）。
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        format.setAlphaBufferSize(8);
        QSurfaceFormat::setDefaultFormat(format);
    }
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("ImageNodeEditor");
    QCoreApplication::setOrganizationName("ALPDHomework");
    QCoreApplication::setApplicationVersion("0.1.0");
    app.setWindowIcon(AppIcon::makeAppIcon());
    // 先按保存的主题设置全局原生外观，确保浅色主题启动时窗口栏/菜单即为浅色
    // （MainWindow 构造时会再次读取同一设置，幂等）。
    AppTheme::setThemePreference(QSettings().value("mainWindow/theme", "dark").toString());
    NativeWindowChrome::applyGlobalAppearance();
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
