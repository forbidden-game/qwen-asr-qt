#pragma once

#include <QDateTime>
#include <QString>

struct HistoryItem {
    QDateTime createdAt;
    QString text;
    QString rawText;
    int elapsedMs = 0;
};
