# AppImage Packaging Notes

This directory is the placeholder for the first public binary release pipeline.

The target artifact is:

```text
qwen-asr-qt-x86_64.AppImage
```

The lightweight AppImage should bundle:

- `qwen-asr-qt`
- `qwen_asr_server`
- runtime libraries needed by the Qt/KDE app
- license and notice files

It should not bundle safetensors model files by default. The app should download
or import model files on first run.

Expected model location:

```text
~/.local/share/qwen-asr-qt/models/Qwen/Qwen3-ASR-0.6B/
```

Future build script outline:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQWEN_ASR_BACKEND_SOURCE_DIR=/path/to/qwen-asr
cmake --build build -j
cmake --install build --prefix AppDir/usr
# copy desktop/icon/NOTICE files
# run linuxdeploy/appimagetool
```

Do not publish a release artifact until it has been tested on a clean KDE user
profile.
