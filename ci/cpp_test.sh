#!/usr/bin/env bash

# This file runs the C++ tests, as well as compiling the code with warnings on
# so that errors should be caught quicker. Extra options can be passed in via $1
# i.e. : ./cpp_test.sh "-DMORPHIO_USE_DOUBLE=ON"

set -euxo pipefail

BUILD_DIR=build/cpp_test

if [ $# -ge 1 ]; then
  EXTRA_OPTIONS=$1
else
  EXTRA_OPTIONS=""
fi

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
pushd $BUILD_DIR
cmake -DMorphIO_CXX_WARNINGS=ON -G "${CMAKE_GENERATOR:-Unix Makefiles}" ${EXTRA_OPTIONS} ../..
cmake --build . -j
ctest -VV
popd
