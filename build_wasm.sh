#!/usr/bin/env bash
# Build the WASM module (web/geo.js + web/geo.wasm) from the C++ core.
#
# Requires an active emsdk in the shell:
#   source /path/to/emsdk/emsdk_env.sh
# emcc/emcmake are NOT part of the base toolchain here — install emsdk first
# (https://emscripten.org/docs/getting_started/downloads.html).
set -euo pipefail

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Activate emsdk: source <emsdk>/emsdk_env.sh" >&2
  exit 1
fi

emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release
cmake --build build-wasm -j

mkdir -p web
cp build-wasm/geo.js build-wasm/geo.wasm web/
echo "Built web/geo.js + web/geo.wasm"
