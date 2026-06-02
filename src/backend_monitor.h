#pragma once

#include "types.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>

class BackendMonitor : public QObject {
    Q_OBJECT

public:
    explicit BackendMonitor(AsrConfig config, QObject *parent = nullptr);

    void setConfig(const AsrConfig &config);
    void start();
    void checkNow();
    BackendState state() const;

signals:
    void backendStateChanged(BackendState state);

private:
    void setState(BackendState state);
    void checkModels();

    AsrConfig config_;
    QNetworkAccessManager network_;
    QTimer timer_;
    BackendState state_ = BackendState::Unknown;
    bool checking_ = false;
};
