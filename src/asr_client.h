#pragma once

#include "types.h"

#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QObject>

class AsrClient : public QObject {
    Q_OBJECT

public:
    explicit AsrClient(AsrConfig config, QObject *parent = nullptr);

    void setConfig(const AsrConfig &config);
    void transcribe(const QString &wavPath);

signals:
    void finished(const HistoryItem &item);
    void errorOccurred(const QString &message);

private:
    QString cleanTranscript(const QString &raw) const;
    QString removeFillers(const QString &text) const;

    AsrConfig config_;
    QNetworkAccessManager network_;
};
