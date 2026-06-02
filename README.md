# Qwen ASR Qt

Qwen ASR Qt is a Linux/KDE-first tray app for local speech-to-text with
Qwen3-ASR through llama.cpp.

Hold `Meta+Space` to record. Release it to transcribe. The cleaned transcript is
copied to the clipboard through Qt and KDE Klipper, so it works reliably on KDE
Wayland.

## Status

This repository is a developer preview. The app is usable today, but the first
"open the package and use it" release still needs an integrated backend/model
manager.

Current behavior:

- Qt tray app with global shortcut through KDE GlobalAccel.
- Live recording overlay with microphone-volume waveform.
- Local HTTP ASR request to llama.cpp `/v1/audio/transcriptions`.
- Transcript cleanup for hesitation fillers and punctuation artifacts.
- Clipboard write through Qt Clipboard plus KDE Klipper DBus.
- History persisted under the user's app data directory.

Planned release behavior:

- App starts and supervises `llama-server`.
- App downloads or imports Qwen3-ASR GGUF files on first run.
- Status panel shows model download, backend startup, model loading, shortcut,
  microphone, and clipboard diagnostics.
- AppImage release for Linux/KDE.

## Requirements

Runtime:

- Linux with KDE Plasma recommended.
- A microphone supported by Qt Multimedia.
- KDE GlobalAccel for the global shortcut.
- KDE Klipper for robust clipboard integration.
- llama.cpp `llama-server`.
- Qwen3-ASR GGUF model and mmproj files.

Build:

```sh
sudo apt install cmake qt6-base-dev qt6-multimedia-dev libkf6globalaccel-dev extra-cmake-modules
```

## Model Files

The app uses these GGUF files:

- `Qwen3-ASR-0.6B-Q8_0.gguf`
- `mmproj-Qwen3-ASR-0.6B-Q8_0.gguf`

Download them with:

```sh
scripts/download-models.sh
```

By default they are placed under:

```text
~/.local/share/qwen-asr-qt/models/ggml-org/Qwen3-ASR-0.6B-GGUF
```

Override with:

```sh
MODEL_DIR=/path/to/models scripts/download-models.sh
```

## Backend

Start llama.cpp with:

```sh
LLAMA_SERVER=/path/to/llama-server scripts/run-backend.sh
```

Optional overrides:

```sh
PORT=18080 MODEL_DIR=/path/to/models THREADS=12 scripts/run-backend.sh
```

The Qt app expects the backend at `http://127.0.0.1:18080`.

## Build And Run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/qwen-asr-qt
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
compatible `llama-server` binary. The AppImage should download the GGUF model on
first run. A separate full/offline artifact can bundle the model later, but it
will be roughly 1 GB larger.

See:

- [Design](docs/design.md)
- [Development](docs/development.md)
- [Packaging](docs/packaging.md)
