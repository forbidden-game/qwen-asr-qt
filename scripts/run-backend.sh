#!/usr/bin/env bash
set -euo pipefail

MODEL_REPO="${MODEL_REPO:-Qwen/Qwen3-ASR-0.6B}"
MODEL_DIR="${MODEL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/qwen-asr-qt/models/$MODEL_REPO}"
QWEN_ASR_SERVER="${QWEN_ASR_SERVER:-qwen_asr_server}"
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-18080}"
THREADS="${THREADS:-12}"
IDLE_UNLOAD_SEC="${IDLE_UNLOAD_SEC:-600}"
MAX_BODY_MB="${MAX_BODY_MB:-128}"
MAX_AUDIO_SEC="${MAX_AUDIO_SEC:-600}"

usage() {
  cat <<'EOF'
Usage: scripts/run-backend.sh

Starts qwen_asr_server for Qwen3-ASR.

Environment:
  QWEN_ASR_SERVER   Path to qwen_asr_server, default qwen_asr_server from PATH
  MODEL_REPO        Hugging Face repo name, default Qwen/Qwen3-ASR-0.6B
  MODEL_DIR         Directory containing safetensors model files
  HOST              Default 127.0.0.1
  PORT              Default 18080
  THREADS           Default 12
  IDLE_UNLOAD_SEC   Default 600 (10 minutes); -1 disables unload
  MAX_BODY_MB       Default 128
  MAX_AUDIO_SEC     Default 600; 0 disables
EOF
}

case "${1:-}" in
  --help|-h)
    usage
    exit 0
    ;;
  "")
    ;;
  *)
    usage >&2
    exit 2
    ;;
esac

if ! command -v "$QWEN_ASR_SERVER" >/dev/null 2>&1 && [[ ! -x "$QWEN_ASR_SERVER" ]]; then
  if [[ -x "./build/qwen_asr_server" ]]; then
    QWEN_ASR_SERVER="./build/qwen_asr_server"
  elif [[ -x "./third_party/qwen-asr/qwen_asr_server" ]]; then
    QWEN_ASR_SERVER="./third_party/qwen-asr/qwen_asr_server"
  else
    echo "Cannot find executable qwen_asr_server: $QWEN_ASR_SERVER" >&2
    echo "Set QWEN_ASR_SERVER=/path/to/qwen_asr_server or configure CMake with QWEN_ASR_BACKEND_SOURCE_DIR." >&2
    exit 1
  fi
fi

missing=0
for file in config.json generation_config.json vocab.json merges.txt; do
  if [[ ! -f "$MODEL_DIR/$file" ]]; then
    echo "Missing model file: $MODEL_DIR/$file" >&2
    missing=1
  fi
done
if [[ ! -f "$MODEL_DIR/model.safetensors" && ! -f "$MODEL_DIR/model.safetensors.index.json" ]]; then
  echo "Missing model weights under: $MODEL_DIR" >&2
  missing=1
fi
if [[ "$missing" == "1" ]]; then
  echo "Run scripts/download-models.sh first, or set MODEL_DIR." >&2
  exit 1
fi

exec "$QWEN_ASR_SERVER" \
  -d "$MODEL_DIR" \
  --alias qwen3-asr \
  --host "$HOST" \
  --port "$PORT" \
  -t "$THREADS" \
  --idle-unload-sec "$IDLE_UNLOAD_SEC" \
  --max-body-mb "$MAX_BODY_MB" \
  --max-audio-sec "$MAX_AUDIO_SEC"
