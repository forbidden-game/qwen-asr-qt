#!/usr/bin/env bash
set -euo pipefail

MODEL_CHOICE="small"
MODEL_REPO="${MODEL_REPO:-}"
MODEL_DIR="${MODEL_DIR:-}"
HF_ENDPOINT="${HF_ENDPOINT:-https://huggingface.co}"

declare -a FILES

usage() {
  cat <<'EOF'
Usage: scripts/download-models.sh [--model small|large] [--dry-run]

Downloads Qwen3-ASR safetensors model files for qwen_asr_server.

Environment:
  MODEL_REPO    Hugging Face repo override
  MODEL_DIR     Destination directory override
  HF_ENDPOINT   Hugging Face endpoint, default https://huggingface.co
EOF
}

dry_run=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --model)
      MODEL_CHOICE="${2:-}"
      shift 2
      ;;
    --dry-run)
      dry_run=1
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      usage >&2
      exit 2
      ;;
  esac
done

case "$MODEL_CHOICE" in
  small|0.6b|0.6B)
    MODEL_REPO="${MODEL_REPO:-Qwen/Qwen3-ASR-0.6B}"
    FILES=(
      "config.json"
      "generation_config.json"
      "model.safetensors"
      "vocab.json"
      "merges.txt"
    )
    ;;
  large|1.7b|1.7B)
    MODEL_REPO="${MODEL_REPO:-Qwen/Qwen3-ASR-1.7B}"
    FILES=(
      "config.json"
      "generation_config.json"
      "model.safetensors.index.json"
      "model-00001-of-00002.safetensors"
      "model-00002-of-00002.safetensors"
      "vocab.json"
      "merges.txt"
    )
    ;;
  *)
    echo "Unknown model: $MODEL_CHOICE" >&2
    usage >&2
    exit 2
    ;;
esac

MODEL_DIR="${MODEL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/qwen-asr-qt/models/$MODEL_REPO}"

if ! command -v curl >/dev/null 2>&1; then
  echo "curl is required" >&2
  exit 1
fi

mkdir -p "$MODEL_DIR"
echo "Model repo: $MODEL_REPO"
echo "Model directory: $MODEL_DIR"

for file in "${FILES[@]}"; do
  url="$HF_ENDPOINT/$MODEL_REPO/resolve/main/$file"
  dest="$MODEL_DIR/$file"
  echo "$file"
  if [[ "$dry_run" == "1" ]]; then
    echo "  would download: $url"
    continue
  fi
  if [[ -s "$dest" ]]; then
    echo "  exists"
    continue
  fi
  tmp="$dest.part"
  curl --fail --location --continue-at - --output "$tmp" "$url"
  mv "$tmp" "$dest"
done

echo "Done."
