#!/usr/bin/env bash
set -o xtrace
BASE=$(git rev-parse --show-toplevel)

if [ "$(docker images | grep morphio_wheel | wc -l)" != "0" ]; then
    echo "Image already exist. Not rebuilding it."

else
    echo "docker image morphio_wheel not found, building it..."
    docker build -t bluebrain/morphio_wheel \
           --build-arg http_proxy=${HTTP_PROXY-$http_proxy} \
           --build-arg https_proxy=${HTTPS_PROXY-$https_proxy} \
           $BASE/packaging/python_wheel | tee build.log

    ID=$(tail -1 build.log | awk '{print $3;}')
    upload2repo -t docker -i $ID -g latest -n morphio_wheel
fi
