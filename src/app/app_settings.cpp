#include "app/app_settings.h"
#include "domain/backend_contract.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace {

constexpr const char *LegacyGgufRepo = "ggml-org/Qwen3-ASR-0.6B-GGUF";

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

bool isLegacyGgufRepo(const QString &repo)
{
    return repo == QString::fromLatin1(LegacyGgufRepo);
}

bool isLegacyGgufModelFile(const QString &file)
{
    return file.endsWith(QStringLiteral(".gguf"), Qt::CaseInsensitive)
        || file.startsWith(QStringLiteral("Qwen3-ASR-0.6B-Q8_0"), Qt::CaseInsensitive);
}

bool isLegacyLlamaServerPath(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }
    const QString fileName = QFileInfo(path).fileName();
    return fileName == QStringLiteral("llama-server");
}

} // namespace

QString ModelSpec::modelPath() const
{
    return QDir(directory).filePath(modelFile);
}

QStringList ModelSpec::requiredFiles() const
{
    return {
        QStringLiteral("config.json"),
        QStringLiteral("generation_config.json"),
        QStringLiteral("model.safetensors"),
        QStringLiteral("vocab.json"),
        QStringLiteral("merges.txt"),
    };
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

    const QString legacyServerPath = settings.value(QStringLiteral("backend/llamaServerPath")).toString();
    QString serverPath = settings.value(
        QStringLiteral("backend/serverPath"),
        legacyServerPath.isEmpty() ? config.backend.serverPath : legacyServerPath).toString();
    if (isLegacyLlamaServerPath(serverPath)) {
        serverPath = config.backend.serverPath;
    }
    config.backend.serverPath = serverPath;

    config.backend.threads = settings.value(QStringLiteral("backend/threads"), config.backend.threads).toInt();
    config.backend.idleUnloadSec = settings.value(QStringLiteral("backend/idleUnloadSec"), config.backend.idleUnloadSec).toInt();
    config.backend.manageProcess = settings.value(QStringLiteral("backend/manageProcess"), config.backend.manageProcess).toBool();

    QString repo = settings.value(QStringLiteral("model/repo"), config.model.repo).toString();
    if (isLegacyGgufRepo(repo)) {
        repo = config.model.repo;
    }
    config.model.repo = repo;

    QString modelFile = settings.value(QStringLiteral("model/modelFile"), config.model.modelFile).toString();
    if (isLegacyGgufModelFile(modelFile)) {
        modelFile = config.model.modelFile;
    }
    config.model.modelFile = modelFile;

    const QString defaultDir = defaultModelDirectory(config.model.repo);
    QString modelDir = settings.value(QStringLiteral("model/directory"), defaultDir).toString();
    if (modelDir.contains(QString::fromLatin1(LegacyGgufRepo)) || modelDir.endsWith(QStringLiteral("Qwen3-ASR-0.6B-GGUF"))) {
        modelDir = defaultDir;
    }
    config.model.directory = modelDir;

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
