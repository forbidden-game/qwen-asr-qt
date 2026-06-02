#pragma once

#include "app/app_settings.h"
#include "domain/asr_types.h"

#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QObject>

class AsrClient : public QObject {
    Q_OBJECT

public:
    explicit AsrClient(AppSettings settings, QObject *parent = nullptr);

    void setSettings(const AppSettings &settings);
    void transcribe(const QString &wavPath);

signals:
    void finished(const HistoryItem &item);
    void errorOccurred(const QString &message);

private:
    AppSettings settings_;
    QNetworkAccessManager network_;
};
