# Packaging

The first public release target is Linux/KDE AppImage.

## Release Shapes

### Lightweight AppImage

Recommended default.

Contents:

- `qwen-asr-qt`
- compatible `qwen_asr_server`
- Qt/KF runtime libraries needed by the app
- third-party notices

Not included:

- Qwen3-ASR safetensors model weights

First run downloads:

- `config.json`
- `generation_config.json`
- `model.safetensors`
- `vocab.json`
- `merges.txt`

This keeps the main artifact small and avoids pushing large model files into
GitHub releases by default.

### Full Offline AppImage

Optional later artifact.

Contents:

- everything in the lightweight AppImage
- Qwen3-ASR safetensors model files

Expected size:

- roughly 2 GB larger than the lightweight artifact for the 0.6B model

## AppImage Layout Draft

```text
AppDir/
  AppRun
  usr/bin/qwen-asr-qt
  usr/bin/qwen_asr_server
  usr/share/applications/qwen-asr-qt.desktop
  usr/share/icons/hicolor/...
  usr/share/qwen-asr-qt/NOTICE
```

Model files should live outside the AppImage by default:

```text
~/.local/share/qwen-asr-qt/models/Qwen/Qwen3-ASR-0.6B/
```

## Dependency Notes

- Qt and KDE Frameworks should be bundled or resolved by the packaging tool.
- `qwen_asr_server` should be built as a CPU baseline binary first.
- The bundled backend should come from a pinned qwen-asr C backend revision.
- GPU-specific variants can be released later as separate artifacts if the
  backend gains them.
- KDE GlobalAccel and Klipper are desktop services; the app should detect when
  they are unavailable and show a clear status.

## Build Tooling To Add

The repo currently includes documentation and helper scripts, not a finished
AppImage build pipeline.

Next packaging work:

1. Add `packaging/appimage/build-appimage.sh`.
2. Build a pinned `qwen_asr_server` binary from the C backend source.
3. Use linuxdeploy or appimagetool to create `AppDir`.
4. Include license texts for bundled binaries.
5. Run the release artifact on a clean KDE user profile.

## Backend Compatibility Policy

The release artifact owns the backend version. Users may point the app at an
external `qwen_asr_server` for development, but the supported path is the
bundled server built from the pinned backend revision.

The app only relies on the documented HTTP contract:

- `POST /v1/audio/transcriptions`
- `GET /health`
- `GET /v1/models`

If the backend changes behavior upstream, existing release artifacts are not
affected because they keep their bundled server. Upgrading the backend should be
a deliberate release task with a contract smoke test.
