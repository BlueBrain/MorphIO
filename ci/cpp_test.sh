#!/usr/bin/env bash

# This file runs the C++ tests, as well as compiling the code with warnings on
# so that errors should be caught quicker. Extra options can be passed in via $1
# i.e. : ./cpp_test.sh "-DMORPHIO_USE_DOUBLE=ON"

set -euxo pipefail

if [ $# -ge 1 ]; then
  EXTRA_OPTIONS=$1
else
  EXTRA_OPTIONS=""
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
  # to use filesystem on macOS:
  # https://stackoverflow.com/questions/57197132/how-to-enable-filesystem-h-or-fix-experimental-filesystem
  EXTRA_OPTIONS="${EXTRA_OPTIONS} -DLIBCXX_INSTALL_FILESYSTEM_LIBRARY=YES"
fi

rm -rf build
mkdir build
pushd build
cmake -DMorphIO_CXX_WARNINGS=ON -G "${CMAKE_GENERATOR:-Unix Makefiles}" ${EXTRA_OPTIONS} -j 2 ..
cmake --build .
ctest -VV
popd
