#include "services/backend_process_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

BackendProcessManager::BackendProcessManager(AppSettings settings, QObject *parent)
    : QObject(parent)
    , settings_(std::move(settings))
{
    process_.setProcessChannelMode(QProcess::SeparateChannels);

    connect(&process_, &QProcess::started, this, [this]() {
        setState(BackendProcessState::Running);
    });
    connect(&process_, &QProcess::readyReadStandardOutput, this, [this]() {
        stdoutBuffer_.append(process_.readAllStandardOutput());
        emitBufferedOutput(stdoutBuffer_);
    });
    connect(&process_, &QProcess::readyReadStandardError, this, [this]() {
        stderrBuffer_.append(process_.readAllStandardError());
        emitBufferedOutput(stderrBuffer_);
    });
    connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        setState(BackendProcessState::Error);
        emit errorOccurred(QStringLiteral("llama-server 启动失败：%1").arg(static_cast<int>(error)));
    });
    connect(&process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (!settings_.backend.manageProcess) {
            setState(BackendProcessState::External);
            return;
        }
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            setState(BackendProcessState::Stopped);
        } else {
            setState(BackendProcessState::Error);
            emit errorOccurred(QStringLiteral("llama-server 已退出：exit=%1").arg(exitCode));
        }
    });
}

BackendProcessManager::~BackendProcessManager()
{
    stop();
}

void BackendProcessManager::setSettings(const AppSettings &settings)
{
    settings_ = settings;
}

void BackendProcessManager::start()
{
    if (!settings_.backend.manageProcess) {
        setState(BackendProcessState::External);
        return;
    }
    if (process_.state() != QProcess::NotRunning) {
        return;
    }

    const QString executable = resolveExecutable();
    if (executable.isEmpty()) {
        setState(BackendProcessState::Error);
        emit errorOccurred(QStringLiteral("找不到固定的 llama-server，请设置 backend/llamaServerPath"));
        return;
    }

    if (!QFileInfo::exists(settings_.model.modelPath()) || !QFileInfo::exists(settings_.model.mmprojPath())) {
        setState(BackendProcessState::Error);
        emit errorOccurred(QStringLiteral("缺少 Qwen3-ASR 模型文件，请先下载或导入模型"));
        return;
    }

    setState(BackendProcessState::Starting);
    process_.setProgram(executable);
    process_.setArguments(arguments());
    process_.start();
}

void BackendProcessManager::stop()
{
    if (process_.state() == QProcess::NotRunning) {
        return;
    }
    process_.terminate();
    if (!process_.waitForFinished(3000)) {
        process_.kill();
        process_.waitForFinished(1000);
    }
}

BackendProcessState BackendProcessManager::state() const
{
    return state_;
}

bool BackendProcessManager::isManaged() const
{
    return settings_.backend.manageProcess;
}

void BackendProcessManager::setState(BackendProcessState state)
{
    if (state_ == state) {
        return;
    }
    state_ = state;
    emit stateChanged(state_);
}

QString BackendProcessManager::resolveExecutable() const
{
    const QFileInfo configured(settings_.backend.llamaServerPath);
    if (configured.isExecutable()) {
        return configured.absoluteFilePath();
    }

    const QString appLocal = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("llama-server"));
    if (QFileInfo(appLocal).isExecutable()) {
        return appLocal;
    }

    return QStandardPaths::findExecutable(settings_.backend.llamaServerPath);
}

QStringList BackendProcessManager::arguments() const
{
    return {
        QStringLiteral("-m"), settings_.model.modelPath(),
        QStringLiteral("--mmproj"), settings_.model.mmprojPath(),
        QStringLiteral("--alias"), settings_.model.alias,
        QStringLiteral("--host"), settings_.backend.host,
        QStringLiteral("--port"), QString::number(settings_.backend.port),
        QStringLiteral("-t"), QString::number(settings_.backend.threads),
        QStringLiteral("-tb"), QString::number(settings_.backend.batchThreads),
        QStringLiteral("-c"), QString::number(settings_.backend.contextSize),
    };
}

void BackendProcessManager::emitBufferedOutput(QByteArray &buffer)
{
    qsizetype newline = buffer.indexOf('\n');
    while (newline >= 0) {
        const QByteArray line = buffer.left(newline).trimmed();
        buffer.remove(0, newline + 1);
        if (!line.isEmpty()) {
            emit logLine(QString::fromUtf8(line));
        }
        newline = buffer.indexOf('\n');
    }
}
