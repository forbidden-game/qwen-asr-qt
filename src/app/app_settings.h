#pragma once

#include <QKeySequence>
#include <QString>
#include <QUrl>

struct ModelSpec {
    QString repo = QStringLiteral("ggml-org/Qwen3-ASR-0.6B-GGUF");
    QString modelFile = QStringLiteral("Qwen3-ASR-0.6B-Q8_0.gguf");
    QString mmprojFile = QStringLiteral("mmproj-Qwen3-ASR-0.6B-Q8_0.gguf");
    QString alias = QStringLiteral("qwen3-asr");
    QString sha256Model;
    QString sha256Mmproj;
};

struct BackendSpec {
    QString llamaServerPath = QStringLiteral("llama-server");
    QString host = QStringLiteral("127.0.0.1");
    int port = 18080;
    int threads = 12;
    int batchThreads = 16;
    int contextSize = 4096;

    QUrl baseUrl() const;
    QUrl transcriptionEndpoint() const;
    QUrl healthUrl() const;
    QUrl modelsUrl() const;
};

struct AudioSpec {
    int sampleRate = 16000;
    int channels = 1;
};

struct TranscriptSpec {
    QString language = QStringLiteral("Chinese");
    QString prompt = QStringLiteral("Transcribe the audio as clean Chinese text. Do not output hesitation fillers or pause tokens such as 嗯, 嗯嗯, 呃, 额, 唔, uh, um, er, unless the speaker explicitly quotes or discusses the filler word. Keep punctuation natural.");
};

struct AppSettings {
    ModelSpec model;
    BackendSpec backend;
    AudioSpec audio;
    TranscriptSpec transcript;
    QKeySequence shortcut = QKeySequence(QStringLiteral("Meta+Space"));
};

AppSettings loadAppSettings();
void saveShortcutSetting(const QKeySequence &shortcut);
