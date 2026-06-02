# Design

Qwen ASR Qt has one product loop:

```text
hold shortcut -> record audio -> transcribe locally -> clean text -> clipboard
```

The code is organized around that loop instead of around individual Qt classes.

## Source Layout

```text
src/
  app/        application state, settings, orchestration
  domain/     transcript and ASR-domain data/cleanup
  services/   audio recording, ASR HTTP, backend checks, history
  platform/   KDE/desktop integrations
  ui/         tray panel and recording overlay
```

## Data Model

`AppSettings` is the runtime configuration root:

```cpp
struct AppSettings {
    ModelSpec model;
    BackendSpec backend;
    AudioSpec audio;
    TranscriptSpec transcript;
    QKeySequence shortcut;
};
```

`BackendSpec` derives the HTTP contract from one host/port pair:

```cpp
QUrl BackendSpec::transcriptionEndpoint() const;
QUrl BackendSpec::healthUrl() const;
QUrl BackendSpec::modelsUrl() const;
```

`RuntimeStatus` is the shape planned for the first-run setup UI:

```cpp
struct RuntimeStatus {
    SetupState setup;
    AppState app;
    BackendState backend;
    QString message;
    int downloadPercent;
};
```

## Runtime Flow

```text
Meta+Space pressed
  -> AppController::startRecording
  -> WavRecorder starts QAudioSource
  -> RecordingOverlay shows live microphone level

Meta+Space released
  -> AppController::stopRecording
  -> WavRecorder finalizes WAV
  -> AsrClient POSTs /v1/audio/transcriptions
  -> TranscriptCleaner removes filler artifacts
  -> ClipboardWriter writes Qt Clipboard + KDE Klipper
  -> HistoryStore persists result
```

## Component Responsibilities

- `app/AppController`: owns the app flow and state transitions.
- `app/AppSettings`: owns durable user-facing configuration.
- `domain/TranscriptCleaner`: cleans prompt echoes, fillers, and punctuation
  artifacts.
- `services/WavRecorder`: records WAV files and emits live input levels.
- `services/AsrClient`: talks to llama.cpp's OpenAI-compatible endpoint.
- `services/BackendMonitor`: checks backend health and model capability.
- `services/HistoryStore`: stores recent transcripts.
- `platform/GlobalShortcut`: registers the KDE global shortcut.
- `platform/ClipboardWriter`: writes Qt Clipboard and KDE Klipper.
- `ui/TrayController`: tray menu, status panel, notifications, history.
- `ui/RecordingOverlay`: KDE-style recording OSD.

## Backend Contract

The current app expects an already-running llama.cpp server:

```text
POST /v1/audio/transcriptions
GET  /health
GET  /v1/models
```

Defaults:

```text
base URL: http://127.0.0.1:18080
model:    qwen3-asr
```

## Next Product Boundary

The public "open and use" release should add:

- `BackendManager`: start/stop bundled `llama-server`, pick a local port, stream
  logs, and recover after crashes.
- `ModelManager`: detect, download, import, and validate GGUF files.
- `SetupPanel`: show missing model, downloading, backend startup, model loading,
  ready, shortcut conflict, microphone error, and clipboard error states.

Until those exist, this repository is a developer preview.
