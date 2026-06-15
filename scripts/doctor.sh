#!/usr/bin/env bash
set -u

MODEL_REPO="${MODEL_REPO:-Qwen/Qwen3-ASR-0.6B}"
MODEL_DIR="${MODEL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/qwen-asr-qt/models/$MODEL_REPO}"
QWEN_ASR_SERVER="${QWEN_ASR_SERVER:-qwen_asr_server}"
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-18080}"

usage() {
  cat <<'EOF'
Usage: scripts/doctor.sh

Checks local build tools, qwen_asr_server, safetensors model files, and backend health.

Environment:
  QWEN_ASR_SERVER   Path/name of qwen_asr_server
  MODEL_REPO        Default Qwen/Qwen3-ASR-0.6B
  MODEL_DIR         Model directory override
  HOST              Backend host, default 127.0.0.1
  PORT              Backend port, default 18080
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

check_executable() {
  local name="$1"
  local exe="$2"
  if command -v "$exe" >/dev/null 2>&1 || [[ -x "$exe" ]] || [[ -x "./build/$exe" ]] || [[ -x "./third_party/qwen-asr/$exe" ]]; then
    printf '[ok]   %s: %s\n' "$name" "$exe"
  else
    printf '[miss] %s: %s\n' "$name" "$exe"
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

check_weights() {
  if [[ -f "$MODEL_DIR/model.safetensors" ]]; then
    printf '[ok]   weights: %s\n' "$MODEL_DIR/model.safetensors"
  elif [[ -f "$MODEL_DIR/model.safetensors.index.json" ]]; then
    printf '[ok]   weights index: %s\n' "$MODEL_DIR/model.safetensors.index.json"
  else
    printf '[miss] weights: %s/model.safetensors or model.safetensors.index.json\n' "$MODEL_DIR"
    failures=$((failures + 1))
  fi
}

echo "Qwen ASR Qt doctor"
echo "Model dir: $MODEL_DIR"

check "cmake" command -v cmake
check "curl" command -v curl
check "qdbus6" command -v qdbus6
check "wl-paste" command -v wl-paste
check_executable "backend" "$QWEN_ASR_SERVER"
check_file "config" "$MODEL_DIR/config.json"
check_file "generation config" "$MODEL_DIR/generation_config.json"
check_file "vocab" "$MODEL_DIR/vocab.json"
check_file "merges" "$MODEL_DIR/merges.txt"
check_weights

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
