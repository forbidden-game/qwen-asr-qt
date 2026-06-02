#include "app/app_settings.h"
#include "domain/backend_contract.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace {

QString defaultModelDirectory(const QString &repo)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(base).filePath(QStringLiteral("models/%1").arg(repo));
}

QUrl withPath(const BackendSpec &backend, const QString &path)
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(backend.host);
    url.setPort(backend.port);
    url.setPath(path);
    return url;
}

} // namespace

QString ModelSpec::modelPath() const
{
    return QDir(directory).filePath(modelFile);
}

QString ModelSpec::mmprojPath() const
{
    return QDir(directory).filePath(mmprojFile);
}

QUrl BackendSpec::baseUrl() const
{
    return withPath(*this, QStringLiteral("/"));
}

QUrl BackendSpec::transcriptionEndpoint() const
{
    return withPath(*this, BackendContract::audioTranscriptionsPath());
}

QUrl BackendSpec::healthUrl() const
{
    return withPath(*this, BackendContract::healthPath());
}

QUrl BackendSpec::modelsUrl() const
{
    return withPath(*this, BackendContract::modelsPath());
}

AppSettings loadAppSettings()
{
    QSettings settings;
    AppSettings config;

    config.backend.host = settings.value(QStringLiteral("backend/host"), config.backend.host).toString();
    config.backend.port = settings.value(QStringLiteral("backend/port"), config.backend.port).toInt();
    config.backend.llamaServerPath = settings.value(QStringLiteral("backend/llamaServerPath"), config.backend.llamaServerPath).toString();
    config.backend.threads = settings.value(QStringLiteral("backend/threads"), config.backend.threads).toInt();
    config.backend.batchThreads = settings.value(QStringLiteral("backend/batchThreads"), config.backend.batchThreads).toInt();
    config.backend.contextSize = settings.value(QStringLiteral("backend/contextSize"), config.backend.contextSize).toInt();
    config.backend.manageProcess = settings.value(QStringLiteral("backend/manageProcess"), config.backend.manageProcess).toBool();

    config.model.repo = settings.value(QStringLiteral("model/repo"), config.model.repo).toString();
    config.model.directory = settings.value(QStringLiteral("model/directory"), defaultModelDirectory(config.model.repo)).toString();
    config.model.modelFile = settings.value(QStringLiteral("model/modelFile"), config.model.modelFile).toString();
    config.model.mmprojFile = settings.value(QStringLiteral("model/mmprojFile"), config.model.mmprojFile).toString();
    config.model.alias = settings.value(QStringLiteral("model/alias"), config.model.alias).toString();

    config.transcript.language = settings.value(QStringLiteral("transcript/language"), config.transcript.language).toString();
    config.transcript.prompt = settings.value(QStringLiteral("transcript/prompt"), config.transcript.prompt).toString();
    config.audio.sampleRate = settings.value(QStringLiteral("audio/sampleRate"), config.audio.sampleRate).toInt();
    config.audio.channels = settings.value(QStringLiteral("audio/channels"), config.audio.channels).toInt();
    config.shortcut = QKeySequence(settings.value(QStringLiteral("shortcut"), config.shortcut.toString()).toString());
    return config;
}

void saveShortcutSetting(const QKeySequence &shortcut)
{
    QSettings settings;
    settings.setValue(QStringLiteral("shortcut"), shortcut.toString());
}
