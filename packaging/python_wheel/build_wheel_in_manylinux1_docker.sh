#!/usr/bin/env bash
set -o xtrace
BASE=$(git rev-parse --show-toplevel)

export PIPPROXY="-i https://bbpteam.epfl.ch/repository/devpi/simple"
export http_proxy=${HTTP_PROXY-$http_proxy}
export https_proxy=${HTTPS_PROXY-$https_proxy}

if [ ! -f $fichier ]; then
    wget https://github.com/Noctem/pogeo-toolchain/releases/download/v1.3/gcc-7.1-binutils-2.28-centos5-x86-64.tar.bz2
    tar -xjvf gcc-7.1-binutils-2.28-centos5-x86-64.tar.bz2
fi

docker images
docker run  \
    -e http_proxy=$http_proxy \
    -e https_proxy=$https_proxy \
    -e PIPPROXY="$PIPPROXY" \
    -v $BASE:/io:Z \
    --user $UID \
    bbpdocker.epfl.ch/morphio_wheel \
    /bin/bash /io/packaging/python_wheel/build_wheel.sh


# Build sdist
cd /io
python setup.py sdist
