#pragma once

#include "app/app_settings.h"
#include "app/app_state.h"

#include <QObject>
#include <QProcess>

class BackendProcessManager : public QObject {
    Q_OBJECT

public:
    explicit BackendProcessManager(AppSettings settings, QObject *parent = nullptr);
    ~BackendProcessManager() override;

    void setSettings(const AppSettings &settings);
    void start();
    void stop();
    BackendProcessState state() const;
    bool isManaged() const;

signals:
    void stateChanged(BackendProcessState state);
    void errorOccurred(const QString &message);
    void logLine(const QString &line);

private:
    void setState(BackendProcessState state);
    QString resolveExecutable() const;
    QStringList arguments() const;
    void emitBufferedOutput(QByteArray &buffer);

    AppSettings settings_;
    QProcess process_;
    BackendProcessState state_ = BackendProcessState::External;
    QByteArray stdoutBuffer_;
    QByteArray stderrBuffer_;
};
