# Development

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Install smoke test:

```sh
cmake --install build --prefix /tmp/qwen-asr-qt-install
```

## Local Model And Backend

Download model files:

```sh
scripts/download-models.sh
```

Start llama.cpp:

```sh
LLAMA_SERVER=/path/to/llama-server scripts/run-backend.sh
```

The default app setting is developer mode: `backend/manageProcess=false`. In
that mode the Qt app never starts llama.cpp; it only monitors and calls the HTTP
server you started. Packaged releases can flip this setting and point
`backend/llamaServerPath` at a bundled, pinned `llama-server`.

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

When using a pre-existing Hugging Face cache:

```sh
MODEL_DIR=/path/to/Qwen3-ASR-0.6B-GGUF/snapshot scripts/doctor.sh
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
