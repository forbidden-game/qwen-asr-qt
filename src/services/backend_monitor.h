#pragma once

#include "app/app_settings.h"
#include "app/app_state.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>

class BackendMonitor : public QObject {
    Q_OBJECT

public:
    explicit BackendMonitor(AppSettings settings, QObject *parent = nullptr);

    void setSettings(const AppSettings &settings);
    void start();
    void checkNow();
    BackendState state() const;

signals:
    void backendStateChanged(BackendState state);

private:
    void setState(BackendState state);
    void checkModels();

    AppSettings settings_;
    QNetworkAccessManager network_;
    QTimer timer_;
    BackendState state_ = BackendState::Unknown;
    bool checking_ = false;
};
