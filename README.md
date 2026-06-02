# Qwen ASR Qt

Tray app for local Qwen3-ASR through llama.cpp.

## Backend

Start llama.cpp first:

```sh
/home/eipi10/work/oss/llama.cpp/build-cpu/bin/llama-server \
  -m /home/eipi10/.cache/huggingface/hub/models--ggml-org--Qwen3-ASR-0.6B-GGUF/snapshots/928ab958557df9aa2ef1c93e0e83c7ad0933fae2/Qwen3-ASR-0.6B-Q8_0.gguf \
  --mmproj /home/eipi10/.cache/huggingface/hub/models--ggml-org--Qwen3-ASR-0.6B-GGUF/snapshots/928ab958557df9aa2ef1c93e0e83c7ad0933fae2/mmproj-Qwen3-ASR-0.6B-Q8_0.gguf \
  --alias qwen3-asr \
  --host 127.0.0.1 \
  --port 18080 \
  -t 12 \
  -tb 16 \
  -c 4096
```

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/qwen-asr-qt
```

Default global shortcut: `Meta+Space`. Hold it to record, release it to transcribe and copy. A small KDE-style recording overlay appears on the screen under the cursor while recording, and its waveform reacts to live microphone volume.
