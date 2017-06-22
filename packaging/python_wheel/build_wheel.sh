#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)

export PIPPROXY="-i https://bbpteam.epfl.ch/repository/devpi/simple"
export http_proxy=$HTTP_PROXY
export https_proxy=$HTTPS_PROXY

docker run --rm \
    -e http_proxy=$http_proxy \
    -e https_proxy=$https_proxy \
    -e PIPPROXY="$PIPPROXY" \
    -e UID=$UID \
    -v $BASE:/io:Z \
    bbpdocker.epfl.ch/brain_wheel \
    /bin/bash /io/packaging/python_wheel/docker_build_wheel.sh
