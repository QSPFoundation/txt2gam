#!/bin/sh

set -e

sudo apt-get update
sudo apt-get install -y \
  wget \
  fuse

REL_BUILD_DIR=./build_packages/linux64_AppImage

cmake -S . -B $REL_BUILD_DIR \
  -DAPP_VERSION="$APP_VERSION" \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Release

cmake --build $REL_BUILD_DIR --parallel $(nproc)
DESTDIR=$REL_BUILD_DIR/AppDir cmake --install $REL_BUILD_DIR --strip

cd $REL_BUILD_DIR || echo "Can't switch to $REL_BUILD_DIR" || exit

wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
chmod +x linuxdeploy-x86_64.AppImage

LIB_PATH=$(find ./AppDir -type d -printf ":%p")
LD_LIBRARY_PATH=$LIB_PATH ./linuxdeploy-x86_64.AppImage --appdir ./AppDir --output appimage
