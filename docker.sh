#!/usr/bin/env sh
docker run --rm -v `pwd`:/io quay.io/pypa/manylinux1_x86_64 /io/build_wheel.sh
