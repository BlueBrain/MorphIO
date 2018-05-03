#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)

docker build --no-cache -t morphio_wheel \
    --build-arg http_proxy=${HTTP_PROXY-$http_proxy} \
    --build-arg https_proxy=${HTTPS_PROXY-$https_proxy} \
    $BASE/packaging/python_wheel | tee build.log

ID=$(tail -1 build.log | awk '{print $3;}')
upload2repo -t docker -i $ID -g latest -n morphio_wheel
