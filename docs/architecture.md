# Architecture

Qwen ASR Qt is split into three runtime responsibilities:

1. Qt tray frontend
2. llama.cpp backend server
3. Qwen3-ASR GGUF model files

The frontend is intentionally small. It records audio, sends a finished WAV
segment to llama.cpp, cleans the transcript, and writes the result to the
clipboard. llama.cpp owns model loading and ASR inference.

## Current Runtime Flow

```text
Meta+Space pressed
  -> WavRecorder starts QAudioSource
  -> RecordingOverlay shows live microphone level

Meta+Space released
  -> WavRecorder finalizes WAV
  -> AsrClient POSTs /v1/audio/transcriptions
  -> TranscriptCleaner removes filler artifacts
  -> ClipboardWriter writes Qt Clipboard + KDE Klipper
  -> HistoryStore persists result
```

## Main Components

- `AppController`: wires app state, recording, ASR, history, tray UI, shortcut,
  overlay, and clipboard.
- `WavRecorder`: records WAV files through Qt Multimedia and emits live input
  levels for the overlay.
- `RecordingOverlay`: transparent KDE-style recording OSD.
- `AsrClient`: sends multipart requests to llama.cpp.
- `TranscriptCleaner`: strips prompt echoes, hesitation fillers, and punctuation
  artifacts.
- `ClipboardWriter`: writes through Qt Clipboard and KDE Klipper DBus.
- `BackendMonitor`: checks `/health` and `/v1/models`.
- `GlobalShortcut`: registers the KDE global shortcut through KF6 GlobalAccel.
- `HistoryStore`: stores recent transcripts in app data.

## Backend Contract

The frontend expects an OpenAI-compatible llama.cpp server:

```text
POST /v1/audio/transcriptions
GET  /health
GET  /v1/models
```

The default endpoint is:

```text
http://127.0.0.1:18080
```

The model alias must be:

```text
qwen3-asr
```

## Planned Managed Backend

The public release should add:

- `BackendManager`: starts/stops bundled `llama-server` with `QProcess`, picks a
  free local port, streams logs, and restarts after crashes.
- `ModelManager`: detects, downloads, imports, and validates GGUF model files.
- `SetupDoctor`: checks microphone, shortcut, model files, backend, port, and
  clipboard support.
- richer tray status for first-run setup and error recovery.

Until those are implemented, the repository is a developer preview and users
start `llama-server` with `scripts/run-backend.sh`.
