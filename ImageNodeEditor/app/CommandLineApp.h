#pragma once

#include <QString>
#include <QStringList>

// picdeal 命令行入口：ffmpeg 式线性流水线 + 执行/校验/节点查询 + 类 git 的保存历史回溯。
// 与 GUI 共用核心（NodeFactory / WorkflowGraph / ExecutionEngine / WorkflowSerializer /
// WorkflowHistory），与 GUI 的「进度记录」共享同一套保存历史。
class CommandLineApp {
public:
    // args 不含程序名；首个元素为子命令。返回进程退出码。
    static int run(const QStringList& args);

    // token 是否为已知子命令（供 main 路由判断是否进入命令行模式）。
    static bool isSubcommand(const QString& token);
};
