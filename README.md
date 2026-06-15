# Qwen ASR Qt

Qwen ASR Qt is a Linux/KDE-first tray app for local speech-to-text with
Qwen3-ASR through a lightweight pure-C HTTP backend, `qwen_asr_server`.

Hold `Meta+Space` to record. Release it to transcribe. The cleaned transcript is
copied to the clipboard through Qt and KDE Klipper, so it works reliably on KDE
Wayland.

## Status

This repository is a developer preview. The app is usable today, but the first
"open the package and use it" release still needs an integrated model manager.

Current behavior:

- Qt tray app with global shortcut through KDE GlobalAccel.
- Live recording overlay with microphone-volume waveform.
- Local HTTP ASR request to `qwen_asr_server` `/v1/audio/transcriptions`.
- Process boundary around the C backend: the Qt app does not link the inference
  engine directly.
- Backend lazy-loads the model on first transcription request and unloads it
  after 10 minutes idle by default.
- Transcript cleanup for hesitation fillers and punctuation artifacts.
- Clipboard write through Qt Clipboard plus KDE Klipper DBus.
- History persisted under the user's app data directory.

Planned release behavior:

- App starts and supervises a bundled, pinned `qwen_asr_server`.
- App downloads or imports Qwen3-ASR safetensors files on first run.
- Status panel shows model download, backend startup, model loading, shortcut,
  microphone, and clipboard diagnostics.
- AppImage release for Linux/KDE.

## Requirements

Runtime:

- Linux with KDE Plasma recommended.
- A microphone supported by Qt Multimedia.
- KDE GlobalAccel for the global shortcut.
- KDE Klipper for robust clipboard integration.
- `qwen_asr_server` built from the pure-C Qwen3-ASR backend.
- Qwen3-ASR safetensors model files.

Build:

```sh
sudo apt install cmake qt6-base-dev qt6-multimedia-dev libkf6globalaccel-dev extra-cmake-modules
```

To build the bundled backend from source you also need the C backend's build
dependencies, including OpenBLAS development headers on Linux.

## Model Files

The app uses the original Qwen3-ASR safetensors files:

- `config.json`
- `generation_config.json`
- `model.safetensors`
- `vocab.json`
- `merges.txt`

Download them with:

```sh
scripts/download-models.sh
```

By default they are placed under:

```text
~/.local/share/qwen-asr-qt/models/Qwen/Qwen3-ASR-0.6B
```

Override with:

```sh
MODEL_DIR=/path/to/qwen3-asr-0.6b scripts/download-models.sh
```

## Backend

Start the C backend with:

```sh
QWEN_ASR_SERVER=/path/to/qwen_asr_server scripts/run-backend.sh
```

Optional overrides:

```sh
PORT=18080 MODEL_DIR=/path/to/qwen3-asr-0.6b THREADS=12 IDLE_UNLOAD_SEC=600 scripts/run-backend.sh
```

The Qt app expects the backend at `http://127.0.0.1:18080`.

The backend contract is intentionally small:

```text
GET  /health
GET  /v1/models
POST /v1/audio/transcriptions
```

## Build And Run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/qwen-asr-qt
```

To also build and copy `qwen_asr_server` next to the Qt binary from a local C
backend checkout:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQWEN_ASR_BACKEND_SOURCE_DIR=/path/to/qwen-asr
cmake --build build -j
```

Useful smoke checks:

```sh
./build/qwen-asr-qt --self-test-cleaner
./build/qwen-asr-qt --preview-overlay
scripts/doctor.sh
```

## Publish Plan

The public repository keeps source code and scripts only. Model weights and
private recordings are intentionally ignored.

The first release should ship a lightweight AppImage containing the Qt app and a
compatible `qwen_asr_server` binary. The AppImage should download the
safetensors model on first run. A separate full/offline artifact can bundle the
model later, but it will be roughly 2 GB larger.

See:

- [Design](docs/design.md)
- [Development](docs/development.md)
- [Packaging](docs/packaging.md)
