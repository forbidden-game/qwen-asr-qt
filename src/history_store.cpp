#include "history_store.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

HistoryStore::HistoryStore(QObject *parent)
    : QObject(parent)
{
}

void HistoryStore::load()
{
    QFile file(path());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
    items_.clear();
    for (const QJsonValue &value : array) {
        const QJsonObject object = value.toObject();
        HistoryItem item;
        item.createdAt = QDateTime::fromString(object.value(QStringLiteral("createdAt")).toString(), Qt::ISODate);
        item.text = object.value(QStringLiteral("text")).toString();
        item.rawText = object.value(QStringLiteral("rawText")).toString();
        item.elapsedMs = object.value(QStringLiteral("elapsedMs")).toInt();
        if (!item.text.isEmpty()) {
            items_.append(item);
        }
    }
    emit changed(items_);
}

void HistoryStore::add(const HistoryItem &item)
{
    items_.prepend(item);
    while (items_.size() > 10) {
        items_.removeLast();
    }
    save();
    emit changed(items_);
}

QVector<HistoryItem> HistoryStore::recent() const
{
    return items_;
}

void HistoryStore::save() const
{
    QFile file(path());
    QDir().mkpath(QFileInfo(file).absolutePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QJsonArray array;
    for (const HistoryItem &item : items_) {
        QJsonObject object;
        object.insert(QStringLiteral("createdAt"), item.createdAt.toString(Qt::ISODate));
        object.insert(QStringLiteral("text"), item.text);
        object.insert(QStringLiteral("rawText"), item.rawText);
        object.insert(QStringLiteral("elapsedMs"), item.elapsedMs);
        array.append(object);
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
}

QString HistoryStore::path() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base + QStringLiteral("/history.json");
}
