#!/usr/bin/env bash
set -u

MODEL_REPO="${MODEL_REPO:-ggml-org/Qwen3-ASR-0.6B-GGUF}"
MODEL_DIR="${MODEL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/qwen-asr-qt/models/$MODEL_REPO}"
MODEL_FILE="${MODEL_FILE:-$MODEL_DIR/Qwen3-ASR-0.6B-Q8_0.gguf}"
MMPROJ_FILE="${MMPROJ_FILE:-$MODEL_DIR/mmproj-Qwen3-ASR-0.6B-Q8_0.gguf}"
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-18080}"

failures=0

check() {
  local name="$1"
  shift
  if "$@" >/dev/null 2>&1; then
    printf '[ok]   %s\n' "$name"
  else
    printf '[miss] %s\n' "$name"
    failures=$((failures + 1))
  fi
}

check_file() {
  local name="$1"
  local file="$2"
  if [[ -f "$file" ]]; then
    printf '[ok]   %s: %s\n' "$name" "$file"
  else
    printf '[miss] %s: %s\n' "$name" "$file"
    failures=$((failures + 1))
  fi
}

echo "Qwen ASR Qt doctor"
echo "Model dir: $MODEL_DIR"

check "cmake" command -v cmake
check "curl" command -v curl
check "qdbus6" command -v qdbus6
check "wl-paste" command -v wl-paste
check_file "model" "$MODEL_FILE"
check_file "mmproj" "$MMPROJ_FILE"

if command -v curl >/dev/null 2>&1; then
  if curl -fsS "http://$HOST:$PORT/health" >/dev/null 2>&1; then
    printf '[ok]   backend health: http://%s:%s\n' "$HOST" "$PORT"
  else
    printf '[warn] backend health unavailable: http://%s:%s\n' "$HOST" "$PORT"
  fi
fi

if [[ "$failures" -gt 0 ]]; then
  echo "$failures required check(s) missing."
  exit 1
fi

echo "Required checks passed."
