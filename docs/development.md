# Development

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

To build and bundle the C backend from a local checkout:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DQWEN_ASR_BACKEND_SOURCE_DIR=/home/eipi10/work/oss/qwen-asr
cmake --build build -j
```

When `QWEN_ASR_BACKEND_SOURCE_DIR` points at a qwen-asr C backend tree, CMake
runs `make server` there and copies `qwen_asr_server` next to
`build/qwen-asr-qt`.

Install smoke test:

```sh
cmake --install build --prefix /tmp/qwen-asr-qt-install
```

## Local Model And Backend

Download model files:

```sh
scripts/download-models.sh
```

Start qwen_asr_server:

```sh
QWEN_ASR_SERVER=/path/to/qwen_asr_server scripts/run-backend.sh
```

The default app setting is developer mode: `backend/manageProcess=false`. In
that mode the Qt app never starts the backend; it only monitors and calls the
HTTP server you started. Packaged releases can flip this setting and point
`backend/serverPath` at a bundled `qwen_asr_server`.

Run the app:

```sh
./build/qwen-asr-qt
```

## Checks

```sh
./build/qwen-asr-qt --self-test-cleaner
./build/qwen-asr-qt --preview-overlay
scripts/doctor.sh
```

When using an existing model checkout/cache:

```sh
MODEL_DIR=/path/to/qwen3-asr-0.6b scripts/doctor.sh
```

## Release Readiness

Before publishing or cutting a release:

- `git status --short` contains no private recordings, model files, or build
  output.
- `cmake --build build -j` passes.
- `./build/qwen-asr-qt --self-test-cleaner` passes.
- `scripts/doctor.sh` passes against a working backend.
- Manual `Meta+Space` recording copies text into Klipper/Wayland clipboard.
- `LICENSE` and `NOTICE` cover bundled third-party components.
