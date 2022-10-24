#!/usr/bin/env bash

set -ex

current_arch=$(uname -m)

yarn workspaces focus @luxonis/discovery
echo "Compile discovery..."
cd /app/discovery

# for static linking
mv depthai-core/cmake/Hunter/config.cmake depthai-core/cmake/Hunter/config.cmake.orig
/bin/grep -v 'BUILD_SHARED_LIBS=ON' depthai-core/cmake/Hunter/config.cmake.orig > depthai-core/cmake/Hunter/config.cmake

yarn run compile
mkdir prebuilds || true
cp build/Release/discovery.node prebuilds/discovery.${current_arch}.node
