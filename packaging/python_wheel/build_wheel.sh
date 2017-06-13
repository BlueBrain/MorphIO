#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)

pushd $BASE/build
cmake .. -DCLONE_SUBPROJECTS=ON
popd

mkdir -p $BASE/ext_src
pushd $BASE/ext_src
[[ -e cmake-3.7.2.tar.gz ]] || wget --continue http://cmake.org/files/v3.7/cmake-3.7.2.tar.gz
[[ -e hdf5-1.8.17.tar.bz2 ]] || wget --continue http://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.17/src/hdf5-1.8.17.tar.bz2
[[ -e boost_1_59_0.tar.gz ]] || wget --continue http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.gz
popd

docker run --rm \
    -v $BASE:/io quay.io/pypa/manylinux1_x86_64 \
    /bin/bash /io/packaging/python_wheel/docker_build_wheel.sh
