#!/bin/sh

export RELEASE_VER="${1:-0.0.0}"

mkdir -p dist

echo "Preparing WASM release $RELEASE_VER"
./build_packages/pack_wasm.sh
