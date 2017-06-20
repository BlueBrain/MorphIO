#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)
SOURCE="http://cmake.org/files/v3.7/cmake-3.7.2.tar.gz \
        http://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.17/src/hdf5-1.8.17.tar.bz2 \
        http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.gz \
        https://github.com/doxygen/doxygen/archive/Release_1_8_5.tar.gz"

pushd $BASE/build
cmake .. -DCLONE_SUBPROJECTS=ON
rm -rf *
popd

mkdir -p $BASE/ext_src
for src in $SOURCE; do
    output=$(basename "$src")
    [[ -e $basename ]] || wget --continue -O $BASE/ext_src/$output $src
done

docker run --rm \
    -v $BASE:/io quay.io/pypa/manylinux1_x86_64 \
    /bin/bash /io/packaging/python_wheel/docker_build_wheel.sh
