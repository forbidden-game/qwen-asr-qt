#!/usr/bin/env bash
set -euo pipefail

MODEL_REPO="${MODEL_REPO:-ggml-org/Qwen3-ASR-0.6B-GGUF}"
MODEL_DIR="${MODEL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/qwen-asr-qt/models/$MODEL_REPO}"
HF_ENDPOINT="${HF_ENDPOINT:-https://huggingface.co}"

FILES=(
  "Qwen3-ASR-0.6B-Q8_0.gguf"
  "mmproj-Qwen3-ASR-0.6B-Q8_0.gguf"
)

usage() {
  cat <<'EOF'
Usage: scripts/download-models.sh [--dry-run]

Downloads the Qwen3-ASR GGUF model and mmproj files.

Environment:
  MODEL_REPO    Hugging Face repo, default ggml-org/Qwen3-ASR-0.6B-GGUF
  MODEL_DIR     Destination directory
  HF_ENDPOINT   Hugging Face endpoint, default https://huggingface.co
EOF
}

dry_run=0
case "${1:-}" in
  --help|-h)
    usage
    exit 0
    ;;
  --dry-run)
    dry_run=1
    ;;
  "")
    ;;
  *)
    usage >&2
    exit 2
    ;;
esac

if ! command -v curl >/dev/null 2>&1; then
  echo "curl is required" >&2
  exit 1
fi

mkdir -p "$MODEL_DIR"
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
