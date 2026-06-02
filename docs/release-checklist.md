# Release Checklist

Use this before creating a public release.

## Repository

- [ ] `git status --short` is clean except intentionally ignored local files.
- [ ] No private recordings, model files, or build output are tracked.
- [ ] `LICENSE` is present.
- [ ] `NOTICE` lists third-party components.
- [ ] README explains current limitations.
- [ ] Docs cover architecture and packaging.

## Validation

- [ ] `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- [ ] `cmake --build build -j`
- [ ] `./build/qwen-asr-qt --self-test-cleaner`
- [ ] `./build/qwen-asr-qt --preview-overlay`
- [ ] `scripts/doctor.sh`
- [ ] Manual `Meta+Space` recording test.
- [ ] Clipboard verified with `wl-paste` or Klipper.

## Runtime

- [ ] llama.cpp backend starts.
- [ ] `/health` returns `{"status":"ok"}`.
- [ ] `/v1/models` exposes `qwen3-asr` with multimodal capability.
- [ ] ASR result is copied to clipboard.
- [ ] Tray history shows cleaned text.

## GitHub

- [ ] Create a public repository named `qwen-asr-qt`.
- [ ] Push `main`.
- [ ] Add repository description.
- [ ] Add topics: `qt`, `kde`, `asr`, `llama-cpp`, `qwen`, `wayland`.
- [ ] Create first release only after packaging is ready.
