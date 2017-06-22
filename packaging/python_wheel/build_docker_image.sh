#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)

docker build -t brain_wheel \
    --build-arg http_proxy=$http_proxy \
    --build-arg https_proxy=$https_proxy \
    $BASE/packaging/python_wheel | tee build.log

ID=$(tail -1 build.log | awk '{print $3;}')
upload2repo -t docker -i $ID -g latest -n brain_wheel
