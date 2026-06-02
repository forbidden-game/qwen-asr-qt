#pragma once

#include "domain/asr_types.h"

#include <QObject>
#include <QVector>

class HistoryStore : public QObject {
    Q_OBJECT

public:
    explicit HistoryStore(QObject *parent = nullptr);

    void load();
    void add(const HistoryItem &item);
    QVector<HistoryItem> recent() const;

signals:
    void changed(const QVector<HistoryItem> &items);

private:
    void save() const;
    QString path() const;

    QVector<HistoryItem> items_;
};
