#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)

PIPPROXY="-i https://bbpteam.epfl.ch/repository/devpi/simple"
http_proxy=${HTTP_PROXY-$http_proxy}
https_proxy=${HTTPS_PROXY-$https_proxy}

docker images
docker run  \
    -e http_proxy=$http_proxy \
    -e https_proxy=$https_proxy \
    -e PIPPROXY="$PIPPROXY" \
    -v $BASE:/io:Z \
    --user $UID \
    bbpdocker.epfl.ch/morphio_wheel \
    /bin/bash /io/packaging/python_wheel/build_wheel.sh


pushd .
cd $BASE
python setup.py sdist
popd
