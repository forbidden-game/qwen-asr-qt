# Packaging

The first public release target is Linux/KDE AppImage.

## Release Shapes

### Lightweight AppImage

Recommended default.

Contents:

- `qwen-asr-qt`
- compatible `llama-server`
- Qt/KF runtime libraries needed by the app
- third-party notices

Not included:

- GGUF model weights

First run downloads:

- `Qwen3-ASR-0.6B-Q8_0.gguf`
- `mmproj-Qwen3-ASR-0.6B-Q8_0.gguf`

This keeps the main artifact small and avoids pushing large model files into
GitHub releases by default.

### Full Offline AppImage

Optional later artifact.

Contents:

- everything in the lightweight AppImage
- Q8 model and mmproj files

Expected size:

- roughly 1 GB larger than the lightweight artifact

## AppImage Layout Draft

```text
AppDir/
  AppRun
  usr/bin/qwen-asr-qt
  usr/bin/llama-server
  usr/share/applications/qwen-asr-qt.desktop
  usr/share/icons/hicolor/...
  usr/share/qwen-asr-qt/NOTICE
```

Model files should live outside the AppImage by default:

```text
~/.local/share/qwen-asr-qt/models/ggml-org/Qwen3-ASR-0.6B-GGUF/
```

## Dependency Notes

- Qt and KDE Frameworks should be bundled or resolved by the packaging tool.
- `llama-server` should be built as a CPU baseline binary first.
- The bundled `llama-server` must be built from the pinned llama.cpp commit
  recorded in `BackendContract`.
- GPU-specific variants can be released later as separate artifacts.
- KDE GlobalAccel and Klipper are desktop services; the app should detect when
  they are unavailable and show a clear status.

## Build Tooling To Add

The repo currently includes documentation and helper scripts, not a finished
AppImage build pipeline.

Next packaging work:

1. Add `packaging/appimage/build-appimage.sh`.
2. Build or download a pinned llama.cpp release.
3. Use linuxdeploy or appimagetool to create `AppDir`.
4. Include license texts for bundled binaries.
5. Run the release artifact on a clean KDE user profile.

## Backend Compatibility Policy

The release artifact owns the backend version. Users may point the app at an
external `llama-server` for development, but the supported path is the bundled
server built from the pinned llama.cpp commit.

The app only relies on the documented HTTP contract:

- `POST /v1/audio/transcriptions`
- `GET /health`
- `GET /v1/models`

If llama.cpp changes behavior upstream, existing release artifacts are not
affected because they keep their bundled server. Upgrading llama.cpp should be a
deliberate release task with a contract smoke test.
