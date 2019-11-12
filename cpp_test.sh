#!/usr/bin/env bash

# This file runs the C++ tests, as well as compiling the code with warnings on
# so that errors should be caught quicker

set -euxo pipefail

rm -rf build;
mkdir build 
pushd build 
cmake .. -DMorphIO_CXX_WARNINGS=ON
make -j2 
make test
popd
