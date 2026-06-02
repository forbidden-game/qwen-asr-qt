#include "backend_monitor.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

BackendMonitor::BackendMonitor(AsrConfig config, QObject *parent)
    : QObject(parent)
    , config_(std::move(config))
{
    timer_.setInterval(5000);
    connect(&timer_, &QTimer::timeout, this, &BackendMonitor::checkNow);
}

void BackendMonitor::setConfig(const AsrConfig &config)
{
    config_ = config;
}

void BackendMonitor::start()
{
    timer_.start();
    checkNow();
}

void BackendMonitor::checkNow()
{
    if (checking_) {
        return;
    }
    checking_ = true;
    QNetworkReply *reply = network_.get(QNetworkRequest(config_.healthUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray body = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError
            && QJsonDocument::fromJson(body).object().value(QStringLiteral("status")).toString() == QStringLiteral("ok");
        reply->deleteLater();
        if (!ok) {
            checking_ = false;
            setState(BackendState::Disconnected);
            return;
        }
        checkModels();
    });
}

BackendState BackendMonitor::state() const
{
    return state_;
}

void BackendMonitor::setState(BackendState state)
{
    if (state_ == state) {
        return;
    }
    state_ = state;
    emit backendStateChanged(state_);
}

void BackendMonitor::checkModels()
{
    QNetworkReply *reply = network_.get(QNetworkRequest(config_.modelsUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray body = reply->readAll();
        checking_ = false;

        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            setState(BackendState::Disconnected);
            return;
        }

        const QJsonObject root = QJsonDocument::fromJson(body).object();
        const QJsonArray models = root.value(QStringLiteral("models")).toArray();
        for (const QJsonValue &value : models) {
            const QJsonObject model = value.toObject();
            const QString name = model.value(QStringLiteral("name")).toString();
            if (name != config_.model) {
                continue;
            }
            const QJsonArray capabilities = model.value(QStringLiteral("capabilities")).toArray();
            for (const QJsonValue &capability : capabilities) {
                if (capability.toString() == QStringLiteral("multimodal")) {
                    reply->deleteLater();
                    setState(BackendState::Connected);
                    return;
                }
            }
        }

        reply->deleteLater();
        setState(BackendState::ModelMissing);
    });
}
