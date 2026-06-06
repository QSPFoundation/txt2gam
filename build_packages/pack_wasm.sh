#!/bin/sh

set -e

# Validation
[ ! -d "./build_packages" ] && echo "Run this script from the project root directory" && exit
[ -z "$RELEASE_VER" ] && echo "RELEASE_VER isn't specified" && exit

CMAKE_VER=$(echo "$RELEASE_VER" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')

WASM_BUILD_DIR=./build_packages/wasm

mkdir -p $WASM_BUILD_DIR/packages

emcmake cmake -S . -B $WASM_BUILD_DIR \
    -DAPP_VERSION="$CMAKE_VER" \
    -DCPACK_OUTPUT_FILE_PREFIX=$WASM_BUILD_DIR/packages \
    -DCPACK_GENERATOR="ZIP" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build $WASM_BUILD_DIR --parallel $(nproc)
cpack -B $WASM_BUILD_DIR --config $WASM_BUILD_DIR/CPackConfig.cmake

mkdir -p ./dist
cp $WASM_BUILD_DIR/packages/*.zip "./dist/txt2gam-$RELEASE_VER-wasm.zip"
( cd ./dist && sha256sum "txt2gam-$RELEASE_VER-wasm.zip" > "txt2gam-$RELEASE_VER-wasm.zip.sha256" )
