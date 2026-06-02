#pragma once

#include <QDateTime>
#include <QKeySequence>
#include <QString>
#include <QUrl>

enum class AppState {
    Idle,
    Recording,
    Transcribing,
    Done,
    Error,
};

enum class BackendState {
    Unknown,
    Connected,
    Disconnected,
    ModelMissing,
};

struct AsrConfig {
    QUrl endpoint = QUrl(QStringLiteral("http://127.0.0.1:18080/v1/audio/transcriptions"));
    QUrl healthUrl = QUrl(QStringLiteral("http://127.0.0.1:18080/health"));
    QUrl modelsUrl = QUrl(QStringLiteral("http://127.0.0.1:18080/v1/models"));
    QString model = QStringLiteral("qwen3-asr");
    QString language = QStringLiteral("Chinese");
    QString prompt = QStringLiteral("Transcribe the audio as clean Chinese text. Do not output hesitation fillers or pause tokens such as 嗯, 嗯嗯, 呃, 额, 唔, uh, um, er, unless the speaker explicitly quotes or discusses the filler word. Keep punctuation natural.");
    QKeySequence shortcut = QKeySequence(QStringLiteral("Meta+Space"));
    int sampleRate = 16000;
    int channels = 1;
};

struct HistoryItem {
    QDateTime createdAt;
    QString text;
    QString rawText;
    int elapsedMs = 0;
};

inline QString appStateText(AppState state)
{
    switch (state) {
    case AppState::Idle:
        return QStringLiteral("Idle");
    case AppState::Recording:
        return QStringLiteral("Recording");
    case AppState::Transcribing:
        return QStringLiteral("Transcribing");
    case AppState::Done:
        return QStringLiteral("Done");
    case AppState::Error:
        return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}

inline QString backendStateText(BackendState state)
{
    switch (state) {
    case BackendState::Unknown:
        return QStringLiteral("Unknown");
    case BackendState::Connected:
        return QStringLiteral("Connected");
    case BackendState::Disconnected:
        return QStringLiteral("Disconnected");
    case BackendState::ModelMissing:
        return QStringLiteral("ModelMissing");
    }
    return QStringLiteral("Unknown");
}
