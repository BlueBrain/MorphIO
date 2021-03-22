#!/usr/bin/env bash

# This builds coverage information, including HTML output

set -euxo pipefail

BUILD_DIR=build/build-coverage

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake                           \
    -DCMAKE_BUILD_TYPE=Debug    \
    -DHIGHFIVE_EXAMPLES=OFF     \
    -DHIGHFIVE_UNIT_TESTS=OFF   \
    -G"Unix Makefiles"          \
    ../..
make -j all coverage
