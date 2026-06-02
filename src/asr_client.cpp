#include "asr_client.h"
#include "transcript_cleaner.h"

#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

AsrClient::AsrClient(AsrConfig config, QObject *parent)
    : QObject(parent)
    , config_(std::move(config))
{
}

void AsrClient::setConfig(const AsrConfig &config)
{
    config_ = config;
}

void AsrClient::transcribe(const QString &wavPath)
{
    auto *file = new QFile(wavPath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorOccurred(QStringLiteral("无法读取录音文件：%1").arg(file->errorString()));
        file->deleteLater();
        return;
    }

    auto *multi = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    auto addTextPart = [multi](const QByteArray &name, const QString &value) {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader, QString::fromLatin1("form-data; name=\"%1\"").arg(QString::fromLatin1(name)));
        part.setBody(value.toUtf8());
        multi->append(part);
    };

    addTextPart("model", config_.model);
    addTextPart("response_format", QStringLiteral("json"));
    addTextPart("language", config_.language);
    addTextPart("prompt", config_.prompt);
    addTextPart("temperature", QStringLiteral("0"));

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QStringLiteral("form-data; name=\"file\"; filename=\"recording.wav\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("audio/wav"));
    filePart.setBodyDevice(file);
    file->setParent(multi);
    multi->append(filePart);

    QNetworkRequest request(config_.endpoint);
    auto *timer = new QElapsedTimer();
    timer->start();
    QNetworkReply *reply = network_.post(request, multi);
    multi->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, timer]() {
        const QByteArray body = reply->readAll();
        const int elapsedMs = static_cast<int>(timer->elapsed());
        delete timer;

        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QStringLiteral("ASR 请求失败：%1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(body);
        const QJsonObject root = doc.object();
        if (root.contains(QStringLiteral("error"))) {
            const QJsonObject error = root.value(QStringLiteral("error")).toObject();
            emit errorOccurred(error.value(QStringLiteral("message")).toString(QStringLiteral("ASR 返回错误")));
            reply->deleteLater();
            return;
        }

        const QString raw = root.value(QStringLiteral("text")).toString();
        if (raw.isEmpty()) {
            emit errorOccurred(QStringLiteral("ASR 返回为空"));
            reply->deleteLater();
            return;
        }

        HistoryItem item;
        item.createdAt = QDateTime::currentDateTime();
        item.rawText = raw;
        item.text = TranscriptCleaner::clean(raw);
        if (item.text.isEmpty()) {
            emit errorOccurred(QStringLiteral("ASR 返回无有效文本"));
            reply->deleteLater();
            return;
        }
        item.elapsedMs = elapsedMs;
        emit finished(item);
        reply->deleteLater();
    });
}
