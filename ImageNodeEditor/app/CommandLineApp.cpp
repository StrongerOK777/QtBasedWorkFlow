#include "app/CommandLineApp.h"

#include "app/CommandLineRunner.h"
#include "core/Edge.h"
#include "nodes/ImageNode.h"
#include "nodes/NodeFactory.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowGraph.h"
#include "workflow/WorkflowHistory.h"
#include "workflow/WorkflowSerializer.h"
#include "workflow/WorkflowValidator.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace {

QTextStream& out()
{
    static QTextStream stream(stdout);
    return stream;
}

QTextStream& err()
{
    static QTextStream stream(stderr);
    return stream;
}

bool parseBool(const QString& value)
{
    const QString v = value.trimmed().toLower();
    return v == "true" || v == "1" || v == "yes" || v == "on";
}

// 把命令行给的文件路径转成绝对路径再存入节点：命令行路径相对当前工作目录，而 workflow
// JSON 保存时会按 workflow 文件所在目录解析相对路径——不转绝对会导致 build/--save 后再
// run 时找不到文件。
QString absoluteFile(const QString& path)
{
    return path.isEmpty() ? path : QFileInfo(path).absoluteFilePath();
}

// 是否为「选项标志」：以 - 开头但不是负数（负数作为位置值如 --brightness -20 仍能被收集）。
bool isFlag(const QString& token)
{
    if (!token.startsWith('-')) {
        return false;
    }
    static const QRegularExpression number("^-?\\d");
    return !number.match(token).hasMatch();
}

QString parameterTypeLabel(ParameterType type)
{
    switch (type) {
    case ParameterType::Integer: return "整数";
    case ParameterType::Double: return "小数";
    case ParameterType::Boolean: return "布尔";
    case ParameterType::Text: return "文本";
    case ParameterType::Choice: return "选项";
    case ParameterType::Color: return "颜色";
    case ParameterType::FileOpen: return "输入文件";
    case ParameterType::FileSave: return "输出文件";
    }
    return "文本";
}

// 按参数类型把字符串转成正确的 QVariant 再 setParameter（loadParams 会按类型校验）。
Status setTypedParam(const QSharedPointer<ImageNode>& node, const QString& key, const QString& valueStr)
{
    const NodeParameter* def = nullptr;
    const auto defs = node->parameterDefinitions();
    for (const auto& d : defs) {
        if (d.name == key) {
            def = &d;
            break;
        }
    }
    if (!def) {
        return Status::fail(QString("节点 %1 没有参数：%2").arg(node->typeName(), key));
    }
    QVariant value;
    switch (def->type) {
    case ParameterType::Integer: {
        bool ok = false;
        const int n = valueStr.toInt(&ok);
        if (!ok) {
            return Status::fail(QString("参数 %1 需要整数，收到：%2").arg(key, valueStr));
        }
        value = n;
        break;
    }
    case ParameterType::Double: {
        bool ok = false;
        const double n = valueStr.toDouble(&ok);
        if (!ok) {
            return Status::fail(QString("参数 %1 需要数字，收到：%2").arg(key, valueStr));
        }
        value = n;
        break;
    }
    case ParameterType::Boolean:
        value = parseBool(valueStr);
        break;
    default:
        value = valueStr;
        break;
    }
    return node->setParameter(key, value);
}

struct OpSpec {
    QString typeName;
    QString primaryParam;     // 裸位置值 → 此参数（空表示不接受裸值）
    bool secondImage = false; // 裸位置值是第二/第三路图片路径（blend/merge）
    bool resizeShorthand = false;  // 支持 WxH 简写
};

const QMap<QString, OpSpec>& opTable()
{
    static const QMap<QString, OpSpec> table = {
        {"crop", {"Crop", "", false, false}},
        {"resize", {"Resize", "", false, true}},
        {"grayscale", {"Grayscale", "", false, false}},
        {"gray", {"Grayscale", "", false, false}},
        {"brightness", {"BrightnessContrast", "brightness", false, false}},
        {"bc", {"BrightnessContrast", "brightness", false, false}},
        {"blur", {"Blur", "radius", false, false}},
        {"rotate", {"RotateFlip", "angle", false, false}},
        {"rotateflip", {"RotateFlip", "angle", false, false}},
        {"text", {"TextOverlay", "text", false, false}},
        {"preview", {"Preview", "", false, false}},
        {"blend", {"Blend", "", true, false}},
        {"merge", {"ImageMerge", "", true, false}},
    };
    return table;
}

// 创建一个操作节点，接入流水线（tail.output → 本节点首个输入），处理 key=value / 位置参数。
// 返回新的流水线尾节点 id。
Result<QString> addOpNode(WorkflowGraph& graph, const QString& op, const QStringList& args, const QString& tail)
{
    if (tail.isEmpty()) {
        return Result<QString>::fail(QString("--%1 之前需要先用 -i 指定输入图片").arg(op));
    }
    if (!opTable().contains(op)) {
        return Result<QString>::fail(QString("未知操作：--%1（用 picdeal nodes 查看可用节点）").arg(op));
    }
    const OpSpec spec = opTable().value(op);
    auto created = NodeFactory::instance().create(spec.typeName);
    if (created.isFail()) {
        return Result<QString>::fail(created.error());
    }
    const QSharedPointer<ImageNode> node = created.value();
    const QString id = graph.addNode(node);
    const QVector<PortInfo> inputs = node->inputPorts();
    if (inputs.isEmpty()) {
        return Result<QString>::fail(QString("节点 %1 没有输入端口，无法接入流水线").arg(spec.typeName));
    }
    const Status edge = graph.addEdge(Edge{tail, "output", id, inputs.first().name});
    if (edge.isFail()) {
        return Result<QString>::fail(edge.error());
    }

    int secondaryPort = 1;
    for (const QString& arg : args) {
        const int eq = arg.indexOf('=');
        if (eq > 0) {
            const Status st = setTypedParam(node, arg.left(eq), arg.mid(eq + 1));
            if (st.isFail()) {
                return Result<QString>::fail(st.error());
            }
            continue;
        }
        if (spec.secondImage) {
            if (secondaryPort >= inputs.size()) {
                return Result<QString>::fail(QString("--%1 输入图片过多（最多 %2 路）").arg(op).arg(inputs.size()));
            }
            auto src = NodeFactory::instance().create("ImageInput");
            if (src.isFail()) {
                return Result<QString>::fail(src.error());
            }
            const Status sp = src.value()->setParameter("filePath", absoluteFile(arg));
            if (sp.isFail()) {
                return Result<QString>::fail(sp.error());
            }
            const QString sid = graph.addNode(src.value());
            const Status se = graph.addEdge(Edge{sid, "output", id, inputs.at(secondaryPort).name});
            if (se.isFail()) {
                return Result<QString>::fail(se.error());
            }
            ++secondaryPort;
            continue;
        }
        if (spec.resizeShorthand) {
            static const QRegularExpression re("^(\\d+)x(\\d+)$");
            const auto m = re.match(arg);
            if (m.hasMatch()) {
                Status sw = setTypedParam(node, "width", m.captured(1));
                if (sw.isFail()) return Result<QString>::fail(sw.error());
                Status sh = setTypedParam(node, "height", m.captured(2));
                if (sh.isFail()) return Result<QString>::fail(sh.error());
                continue;
            }
        }
        if (!spec.primaryParam.isEmpty()) {
            const Status st = setTypedParam(node, spec.primaryParam, arg);
            if (st.isFail()) {
                return Result<QString>::fail(st.error());
            }
            continue;
        }
        return Result<QString>::fail(QString("--%1 不接受位置参数「%2」，请用 key=value 形式").arg(op, arg));
    }
    return Result<QString>::ok(id);
}

// 把 ffmpeg 式参数序列构建为一张线性工作流图。savePath（如有 --save）写回。返回尾节点 id。
Result<QString> buildPipeline(WorkflowGraph& graph, const QStringList& tokens, QString* savePath)
{
    QString tail;
    int i = 0;
    while (i < tokens.size()) {
        const QString t = tokens.at(i);
        if (t == "-i" || t == "--input") {
            if (i + 1 >= tokens.size()) return Result<QString>::fail("-i 缺少图片路径");
            const QString path = tokens.at(i + 1);
            i += 2;
            if (!tail.isEmpty()) {
                return Result<QString>::fail("pipe 仅支持单一输入起点；多输入/分支请改用 workflow.json");
            }
            auto created = NodeFactory::instance().create("ImageInput");
            if (created.isFail()) return Result<QString>::fail(created.error());
            const Status sp = created.value()->setParameter("filePath", absoluteFile(path));
            if (sp.isFail()) return Result<QString>::fail(sp.error());
            tail = graph.addNode(created.value());
            continue;
        }
        if (t == "-o" || t == "--output") {
            if (i + 1 >= tokens.size()) return Result<QString>::fail("-o 缺少输出路径");
            const QString path = tokens.at(i + 1);
            i += 2;
            if (tail.isEmpty()) return Result<QString>::fail("-o 之前需要先有 -i 输入和处理步骤");
            auto created = NodeFactory::instance().create("ImageOutput");
            if (created.isFail()) return Result<QString>::fail(created.error());
            const Status sp = created.value()->setParameter("outputPath", absoluteFile(path));
            if (sp.isFail()) return Result<QString>::fail(sp.error());
            const QString id = graph.addNode(created.value());
            const Status se = graph.addEdge(Edge{tail, "output", id, created.value()->inputPorts().first().name});
            if (se.isFail()) return Result<QString>::fail(se.error());
            tail = id;
            continue;
        }
        if (t == "--save") {
            if (i + 1 >= tokens.size()) return Result<QString>::fail("--save 缺少文件路径");
            if (savePath) *savePath = tokens.at(i + 1);
            i += 2;
            continue;
        }
        if (t.startsWith("--")) {
            const QString op = t.mid(2);
            QStringList args;
            int j = i + 1;
            while (j < tokens.size() && !isFlag(tokens.at(j))) {
                args << tokens.at(j);
                ++j;
            }
            i = j;
            const auto built = addOpNode(graph, op, args, tail);
            if (built.isFail()) return built;
            tail = built.value();
            continue;
        }
        return Result<QString>::fail(QString("无法识别的参数：%1").arg(t));
    }
    if (tail.isEmpty()) {
        return Result<QString>::fail("空流水线：至少需要 -i 指定输入图片");
    }
    return Result<QString>::ok(tail);
}

QString optionValue(const QStringList& args, const QStringList& names)
{
    for (int i = 0; i < args.size() - 1; ++i) {
        if (names.contains(args.at(i))) {
            return args.at(i + 1);
        }
    }
    return {};
}

QString firstPositional(const QStringList& args)
{
    for (int i = 0; i < args.size(); ++i) {
        const QString& a = args.at(i);
        if (isFlag(a)) {
            // 跳过带值选项的值
            if (a == "-o" || a == "--out" || a == "-m" || a == "--message") {
                ++i;
            }
            continue;
        }
        return a;
    }
    return {};
}

int cmdPipe(const QStringList& args)
{
    WorkflowGraph graph;
    QString savePath;
    const auto built = buildPipeline(graph, args, &savePath);
    if (built.isFail()) {
        err() << "构建流水线失败：" << built.error() << Qt::endl;
        return 1;
    }
    if (!savePath.isEmpty()) {
        const Status saved = WorkflowSerializer().saveFile(graph, savePath);
        if (saved.isFail()) {
            err() << "保存工作流失败：" << saved.error() << Qt::endl;
            return 2;
        }
        out() << "已保存工作流：" << savePath << Qt::endl;
    }
    const Status valid = WorkflowValidator().validate(graph);
    if (valid.isFail()) {
        err() << "校验未通过：" << valid.error() << Qt::endl;
        return 3;
    }
    ExecutionEngine engine;
    const auto result = engine.execute(graph);
    if (result.isFail()) {
        err() << "执行失败：" << result.error() << Qt::endl;
        return 4;
    }
    for (const QString& line : result.value().log) {
        out() << line << Qt::endl;
    }
    out() << "完成。" << Qt::endl;
    return 0;
}

int cmdBuild(const QStringList& args)
{
    WorkflowGraph graph;
    QString savePath;
    const auto built = buildPipeline(graph, args, &savePath);
    if (built.isFail()) {
        err() << "构建流水线失败：" << built.error() << Qt::endl;
        return 1;
    }
    if (savePath.isEmpty()) {
        err() << "build 需要用 --save <workflow.json> 指定保存路径" << Qt::endl;
        return 1;
    }
    const Status saved = WorkflowSerializer().saveFile(graph, savePath);
    if (saved.isFail()) {
        err() << "保存工作流失败：" << saved.error() << Qt::endl;
        return 2;
    }
    out() << "已保存工作流：" << savePath << "（可在图形界面打开继续编辑）" << Qt::endl;
    return 0;
}

int cmdValidate(const QStringList& args)
{
    const QString path = firstPositional(args);
    if (path.isEmpty()) {
        err() << "validate 需要 workflow.json 路径" << Qt::endl;
        return 1;
    }
    const auto graph = WorkflowSerializer().loadFile(path);
    if (graph.isFail()) {
        err() << "加载失败：" << graph.error() << Qt::endl;
        return 2;
    }
    const Status valid = WorkflowValidator().validate(graph.value());
    if (valid.isFail()) {
        err() << "校验未通过：" << valid.error() << Qt::endl;
        return 3;
    }
    out() << "校验通过：" << path << Qt::endl;
    return 0;
}

int cmdNodes(const QStringList&)
{
    for (const NodeDescriptor& desc : NodeFactory::instance().descriptors()) {
        out() << desc.typeName << "  —  " << desc.displayName << "  [" << desc.category << "]" << Qt::endl;
        const auto created = NodeFactory::instance().create(desc.typeName);
        if (created.isFail()) {
            continue;
        }
        for (const NodeParameter& p : created.value()->parameterDefinitions()) {
            QString line = QString("    %1 (%2)").arg(p.name, parameterTypeLabel(p.type));
            if (p.type == ParameterType::Integer || p.type == ParameterType::Double) {
                if (p.min != p.max) {
                    line += QString(" 范围 %1~%2").arg(p.min).arg(p.max);
                }
            }
            if (!p.options.isEmpty()) {
                line += QString(" 可选 [%1]").arg(p.options.join('/'));
            }
            if (p.defaultValue.isValid() && !p.defaultValue.toString().isEmpty()) {
                line += QString(" 默认 %1").arg(p.defaultValue.toString());
            }
            out() << line << Qt::endl;
        }
    }
    return 0;
}

int cmdRun(const QStringList& args)
{
    const QString path = firstPositional(args);
    if (path.isEmpty()) {
        err() << "run 需要 workflow.json 路径" << Qt::endl;
        return 1;
    }
    return CommandLineRunner().run(path);
}

int cmdSave(const QStringList& args)
{
    const QString path = firstPositional(args);
    if (path.isEmpty()) {
        err() << "save 需要 workflow.json 路径" << Qt::endl;
        return 1;
    }
    const auto graph = WorkflowSerializer().loadFile(path);
    if (graph.isFail()) {
        err() << "加载失败：" << graph.error() << Qt::endl;
        return 2;
    }
    QString message = optionValue(args, {"-m", "--message"});
    if (message.isEmpty()) {
        message = QString("命令行保存 · %1").arg(QFileInfo(path).fileName());
    }
    const auto id = WorkflowHistory::save(WorkflowHistory::kCheckpoints, graph.value(),
                                          {{"title", message}, {"branch", "main"}});
    if (id.isFail()) {
        err() << "保存到历史失败：" << id.error() << Qt::endl;
        return 4;
    }
    out() << "已保存到历史（图形界面「进度记录」可见）：" << id.value() << Qt::endl;
    return 0;
}

int cmdLog(const QStringList& args)
{
    const bool timeline = args.contains("--timeline");
    const QString group = timeline ? WorkflowHistory::kTimeline : WorkflowHistory::kCheckpoints;
    const auto entries = WorkflowHistory::list(group);
    if (entries.isEmpty()) {
        out() << (timeline ? "（暂无自动保存时间线）" : "（暂无保存点；用 picdeal save 或在图形界面保存）") << Qt::endl;
        return 0;
    }
    out() << (timeline ? "保存时间线（最新在前）：" : "保存点（最新在前）：") << Qt::endl;
    for (const WorkflowHistory::Entry& e : entries) {
        const QString when = e.when.toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
        const QString title = e.meta.value("title", e.meta.value("label"));
        const QString branch = e.meta.value("branch");
        out() << "  " << e.id << "  " << when;
        if (!branch.isEmpty()) {
            out() << "  [" << branch << "]";
        }
        if (!title.isEmpty()) {
            out() << "  " << title;
        }
        out() << Qt::endl;
    }
    return 0;
}

int cmdRestore(const QStringList& args)
{
    const QString id = firstPositional(args);
    if (id.isEmpty()) {
        err() << "restore 需要保存 id（用 picdeal log 查看）" << Qt::endl;
        return 1;
    }
    Result<WorkflowGraph> graph = Result<WorkflowGraph>::fail("未找到");
    for (const QString& group : {WorkflowHistory::kCheckpoints, WorkflowHistory::kTimeline}) {
        auto loaded = WorkflowHistory::load(group, id);
        if (loaded.isOk()) {
            graph = loaded;
            break;
        }
    }
    if (graph.isFail()) {
        err() << "找不到保存：" << id << Qt::endl;
        return 2;
    }
    const auto json = WorkflowSerializer().toJson(graph.value());
    if (json.isFail()) {
        err() << "序列化失败：" << json.error() << Qt::endl;
        return 2;
    }
    const QString outPath = optionValue(args, {"-o", "--out"});
    if (outPath.isEmpty()) {
        out() << QString::fromUtf8(json.value()) << Qt::endl;
        return 0;
    }
    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        err() << "无法写入：" << outPath << Qt::endl;
        return 2;
    }
    file.write(json.value());
    file.close();
    out() << "已恢复到：" << outPath << Qt::endl;
    return 0;
}

int cmdHelp(const QStringList&)
{
    out() << R"(picdeal — 节点式图像处理命令行（与图形界面共用核心与保存历史）

用法：
  picdeal pipe -i 输入 [--操作 [键=值|值]...] -o 输出   ffmpeg 式线性流水线并执行
  picdeal build -i ... [--操作 ...] --save 工作流.json    构建并保存为工作流（可在 GUI 打开）
  picdeal run 工作流.json                                 执行已保存的工作流
  picdeal validate 工作流.json                            仅校验工作流
  picdeal nodes                                           列出所有节点与参数
  picdeal save 工作流.json [-m 说明]                      存入保存历史（GUI「进度记录」可见）
  picdeal log [--timeline]                                类 git log 列出保存历史
  picdeal restore <id> [-o 输出.json]                     回溯到某次保存
  picdeal help | version

操作（--后）：grayscale/gray、blur [radius|值]、resize [WxH|width= height= keepAspect=]、
  crop [x= y= width= height=]、brightness/bc [值|brightness= contrast=]、
  rotate [角度|angle= flipHorizontal= flipVertical=]、text [文字|text= x= y= size= color=]、
  preview、blend 第二图 [opacity=]、merge 第二图 [第三图] [mode=horizontal|vertical]

示例：
  picdeal pipe -i in.png --grayscale --blur 3 --resize 800x600 -o out.png
  picdeal pipe -i a.png --blend b.png opacity=0.4 -o merged.png
  picdeal build -i in.png --brightness 20 contrast=1.2 --save flow.json
  picdeal save flow.json -m "初版" && picdeal log
)" << Qt::endl;
    return 0;
}

}  // namespace

bool CommandLineApp::isSubcommand(const QString& token)
{
    static const QStringList subs = {"pipe", "build", "run", "validate", "nodes",
                                     "save", "log", "restore", "help", "version"};
    return subs.contains(token);
}

int CommandLineApp::run(const QStringList& args)
{
    NodeFactory::instance().registerBuiltins();
    if (args.isEmpty()) {
        return cmdHelp({});
    }
    const QString sub = args.first();
    const QStringList rest = args.mid(1);
    if (sub == "help" || sub == "--help" || sub == "-h") {
        return cmdHelp(rest);
    }
    if (sub == "version" || sub == "--version" || sub == "-v") {
        out() << "picdeal 0.1.0" << Qt::endl;
        return 0;
    }
    if (sub == "pipe") return cmdPipe(rest);
    if (sub == "build") return cmdBuild(rest);
    if (sub == "run") return cmdRun(rest);
    if (sub == "validate") return cmdValidate(rest);
    if (sub == "nodes") return cmdNodes(rest);
    if (sub == "save") return cmdSave(rest);
    if (sub == "log") return cmdLog(rest);
    if (sub == "restore") return cmdRestore(rest);

    err() << "未知子命令：" << sub << Qt::endl;
    cmdHelp({});
    return 1;
}
