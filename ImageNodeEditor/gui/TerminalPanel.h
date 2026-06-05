#pragma once

#include "gui/AppTheme.h"

#include <QDir>
#include <QFont>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

// 底部「终端」面板：跨平台命令运行器（非完整 PTY）。macOS/Linux 用 $SHELL，Windows 用
// PowerShell/cmd；支持输入命令、运行、清空、重启。析构时先结束子进程，避免退出期 QProcess 警告。
class TerminalPanel final : public QWidget {
public:
    explicit TerminalPanel(double uiScale, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setObjectName("terminalPanel");
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(AppTheme::px(8, uiScale), AppTheme::px(8, uiScale),
                                 AppTheme::px(8, uiScale), AppTheme::px(8, uiScale));
        root->setSpacing(AppTheme::px(6, uiScale));

        output_ = new QPlainTextEdit;
        output_->setObjectName("terminalOutput");
        output_->setReadOnly(true);
        output_->setLineWrapMode(QPlainTextEdit::NoWrap);
        QFont mono("Menlo");
        mono.setStyleHint(QFont::Monospace);
        mono.setPointSizeF(std::max(10.0, 11.5 * uiScale));
        output_->setFont(mono);
        root->addWidget(output_, 1);

        auto* controls = new QWidget;
        auto* controlsLayout = new QHBoxLayout(controls);
        controlsLayout->setContentsMargins(0, 0, 0, 0);
        controlsLayout->setSpacing(AppTheme::px(6, uiScale));
        input_ = new QLineEdit;
        input_->setPlaceholderText("输入命令后按 Enter");
        auto* runButton = new QPushButton("运行");
        auto* clearButton = new QPushButton("清空");
        auto* restartButton = new QPushButton("重启");
        controlsLayout->addWidget(input_, 1);
        controlsLayout->addWidget(runButton);
        controlsLayout->addWidget(clearButton);
        controlsLayout->addWidget(restartButton);
        root->addWidget(controls);

        process_.setProcessChannelMode(QProcess::MergedChannels);
        process_.setWorkingDirectory(QDir::currentPath());
        connect(&process_, &QProcess::readyReadStandardOutput, this, [this] {
            append(QString::fromLocal8Bit(process_.readAllStandardOutput()));
        });
        connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            Q_UNUSED(error);
            append(QString("终端错误：%1\n").arg(process_.errorString()));
        });
        connect(&process_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus status) {
            Q_UNUSED(status);
            append(QString("\n[进程结束，退出码 %1]\n").arg(code));
        });
        connect(input_, &QLineEdit::returnPressed, this, [this] { submitCommand(); });
        connect(runButton, &QPushButton::clicked, this, [this] { submitCommand(); });
        connect(clearButton, &QPushButton::clicked, output_, &QPlainTextEdit::clear);
        connect(restartButton, &QPushButton::clicked, this, [this] { restartShell(); });

        startShell();
    }

    ~TerminalPanel() override
    {
        // 退出时先结束内嵌 shell 子进程，避免 QProcess 析构时仍在运行而报警告。
        if (process_.state() != QProcess::NotRunning) {
            process_.kill();
            process_.waitForFinished(500);
        }
    }

    void restartShell()
    {
        if (process_.state() != QProcess::NotRunning) {
            process_.kill();
            process_.waitForFinished(800);
        }
        startShell();
    }

private:
    void append(const QString& text)
    {
        output_->moveCursor(QTextCursor::End);
        output_->insertPlainText(text);
        output_->moveCursor(QTextCursor::End);
    }

    void startShell()
    {
        QString program;
        QStringList args;
#ifdef Q_OS_WIN
        program = QStandardPaths::findExecutable("powershell.exe");
        if (!program.isEmpty()) {
            args << "-NoLogo" << "-NoExit";
        } else {
            program = QStandardPaths::findExecutable("cmd.exe");
        }
#else
        program = qEnvironmentVariable("SHELL");
        if (program.isEmpty()) {
            program = QStandardPaths::findExecutable("bash");
        }
        if (program.isEmpty()) {
            program = "/bin/bash";
        }
        args << "-i";
#endif
        append(QString("[启动终端] %1 %2\n").arg(program, args.join(' ')).trimmed() + "\n");
        process_.start(program, args);
    }

    void submitCommand()
    {
        const QString command = input_->text();
        if (command.trimmed().isEmpty()) {
            return;
        }
        if (process_.state() == QProcess::NotRunning) {
            startShell();
        }
        append(QString("> %1\n").arg(command));
        process_.write(command.toLocal8Bit());
        process_.write("\n");
        input_->clear();
    }

    QProcess process_;
    QPlainTextEdit* output_ = nullptr;
    QLineEdit* input_ = nullptr;
};
