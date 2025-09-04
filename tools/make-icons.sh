#!/usr/bin/env bash
set -euo pipefail

SRC=${1:-logo.svg}
[[ -f "$SRC" ]] || { echo "File SVG non trovato: $SRC"; exit 1; }

mkdir -p res assets docs

echo "[1/4] SVG -> PNG sizes"
if command -v rsvg-convert >/dev/null 2>&1; then
  for S in 16 24 32 48 64 128 256; do
    rsvg-convert -w "$S" -h "$S" "$SRC" -o "assets/logo-${S}.png"
  done
else
  echo "rsvg-convert non trovato, uso ImageMagick convert (IM6)"
  for S in 16 24 32 48 64 128 256; do
    convert -density 1024 -background none "$SRC" -resize ${S}x${S} -strip "assets/logo-${S}.png"
  done
fi

echo "[2/4] Build ICO multi-size"
convert assets/logo-16.png assets/logo-24.png assets/logo-32.png \
        assets/logo-48.png assets/logo-64.png assets/logo-128.png assets/logo-256.png \
        res/hexcompose.ico

echo "[3/4] Favicons & aliases"
cp res/hexcompose.ico assets/favicon.ico
cp assets/logo-256.png assets/logo.png

echo "[4/4] Social preview 1280x640"
convert -size 1280x640 xc:"#0b1220" \
        \( assets/logo-256.png -resize 512x512 \) -gravity center -composite \
        -strip docs/social-preview.png

echo "Done. Files in res/, assets/, docs/"
