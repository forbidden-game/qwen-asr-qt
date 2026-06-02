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

The app does not link `libllama`, `ggml`, or `mtmd`. It talks to a separate
`llama-server` process over HTTP. This keeps llama.cpp crashes, upgrades, and
ABI changes outside the Qt process.

The backend contract is intentionally small:

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

Pinned release metadata:

```text
llama.cpp repo:   ggml-org/llama.cpp
llama.cpp commit: 60130d18f9ac7f42cb4d7f6060b088a45d8f242e
local describe:   b9478
```

Developer mode keeps `backend/manageProcess=false`, so the app only monitors an
already-running server. Release mode sets `backend/manageProcess=true` and
starts a bundled, pinned `llama-server` executable with `QProcess`.

## Next Product Boundary

The public "open and use" release should add:

- robust `BackendProcessManager` UI: logs, version display, and restart after
  crashes.
- `ModelManager`: detect, download, import, and validate GGUF files.
- `SetupPanel`: show missing model, downloading, backend startup, model loading,
  ready, shortcut conflict, microphone error, and clipboard error states.

Until those exist, this repository is a developer preview.
