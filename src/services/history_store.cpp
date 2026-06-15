#include "services/history_store.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {

constexpr qsizetype RecentHistoryLimit = 10;

HistoryItem historyItemFromJson(const QJsonObject &object)
{
    HistoryItem item;
    item.createdAt = QDateTime::fromString(object.value(QStringLiteral("createdAt")).toString(), Qt::ISODate);
    item.text = object.value(QStringLiteral("text")).toString();
    item.rawText = object.value(QStringLiteral("rawText")).toString();
    item.elapsedMs = object.value(QStringLiteral("elapsedMs")).toInt();
    return item;
}

QJsonObject historyItemToJson(const HistoryItem &item)
{
    QJsonObject object;
    object.insert(QStringLiteral("createdAt"), item.createdAt.toString(Qt::ISODate));
    object.insert(QStringLiteral("text"), item.text);
    object.insert(QStringLiteral("rawText"), item.rawText);
    object.insert(QStringLiteral("elapsedMs"), item.elapsedMs);
    return object;
}

} // namespace

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
        const HistoryItem item = historyItemFromJson(value.toObject());
        if (!item.text.isEmpty()) {
            items_.append(item);
        }
        if (items_.size() == RecentHistoryLimit) {
            break;
        }
    }
    seedArchiveIfMissing();
    emit changed(items_);
}

void HistoryStore::add(const HistoryItem &item)
{
    appendArchive(item);
    items_.prepend(item);
    while (items_.size() > RecentHistoryLimit) {
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
        array.append(historyItemToJson(item));
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
}

void HistoryStore::appendArchive(const HistoryItem &item) const
{
    QFile file(archivePath());
    QDir().mkpath(QFileInfo(file).absolutePath());

    QJsonArray array;
    if (file.open(QIODevice::ReadOnly)) {
        array = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    array.append(historyItemToJson(item));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
}

void HistoryStore::seedArchiveIfMissing() const
{
    if (items_.isEmpty() || QFileInfo::exists(archivePath())) {
        return;
    }

    QFile file(archivePath());
    QDir().mkpath(QFileInfo(file).absolutePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QJsonArray array;
    for (qsizetype i = items_.size() - 1; i >= 0; --i) {
        array.append(historyItemToJson(items_.at(i)));
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
}

QString HistoryStore::archivePath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base + QStringLiteral("/history-all.json");
}

QString HistoryStore::path() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base + QStringLiteral("/history.json");
}
