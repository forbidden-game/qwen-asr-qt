#!/usr/bin/env bash
set -euo pipefail

MODEL_REPO="${MODEL_REPO:-ggml-org/Qwen3-ASR-0.6B-GGUF}"
MODEL_DIR="${MODEL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/qwen-asr-qt/models/$MODEL_REPO}"
MODEL_FILE="${MODEL_FILE:-$MODEL_DIR/Qwen3-ASR-0.6B-Q8_0.gguf}"
MMPROJ_FILE="${MMPROJ_FILE:-$MODEL_DIR/mmproj-Qwen3-ASR-0.6B-Q8_0.gguf}"
LLAMA_SERVER="${LLAMA_SERVER:-llama-server}"
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-18080}"
THREADS="${THREADS:-$(nproc)}"
BATCH_THREADS="${BATCH_THREADS:-$THREADS}"
CONTEXT="${CONTEXT:-4096}"
PINNED_LLAMA_CPP_COMMIT="${PINNED_LLAMA_CPP_COMMIT:-60130d18f9ac7f42cb4d7f6060b088a45d8f242e}"

usage() {
  cat <<'EOF'
Usage: scripts/run-backend.sh

Starts llama.cpp for Qwen3-ASR.

Environment:
  LLAMA_SERVER   Path to llama-server, default llama-server from PATH
  MODEL_DIR      Directory containing GGUF files
  MODEL_FILE     Full path to Qwen3-ASR GGUF
  MMPROJ_FILE    Full path to mmproj GGUF
  HOST           Default 127.0.0.1
  PORT           Default 18080
  THREADS        Default nproc
  CONTEXT        Default 4096
  PINNED_LLAMA_CPP_COMMIT
                Recommended llama.cpp commit for packaged releases
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

if ! command -v "$LLAMA_SERVER" >/dev/null 2>&1 && [[ ! -x "$LLAMA_SERVER" ]]; then
  echo "Cannot find executable llama-server: $LLAMA_SERVER" >&2
  echo "Set LLAMA_SERVER=/path/to/llama-server" >&2
  exit 1
fi

if [[ ! -f "$MODEL_FILE" || ! -f "$MMPROJ_FILE" ]]; then
  echo "Missing model files under: $MODEL_DIR" >&2
  echo "Run scripts/download-models.sh first, or set MODEL_DIR/MODEL_FILE/MMPROJ_FILE." >&2
  exit 1
fi

exec "$LLAMA_SERVER" \
  -m "$MODEL_FILE" \
  --mmproj "$MMPROJ_FILE" \
  --alias qwen3-asr \
  --host "$HOST" \
  --port "$PORT" \
  -t "$THREADS" \
  -tb "$BATCH_THREADS" \
  -c "$CONTEXT"
