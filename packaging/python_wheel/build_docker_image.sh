#!/usr/bin/env bash
set -euxo pipefail

BASE=$(git rev-parse --show-toplevel)

if [ "$(docker images | grep morphio_wheel | wc -l)" != "0" ]; then
    echo "Image already exist. Not rebuilding it."

else
    echo "docker image morphio_wheel not found, building it..."
    DOCKER_ENV=
    if [[ -v JENKINS_URL ]]; then
       DOCKER_ENV="--build-arg http_proxy=${HTTP_PROXY-$http_proxy}"
       DOCKER_ENV+="--build-arg https_proxy=${HTTPS_PROXY-$https_proxy}"
    fi

    docker build -t bluebrain/morphio_wheel                     \
           $DOCKER_ENV                                          \
           $BASE/packaging/python_wheel                         \
           | tee build.log

    if [[ -v JENKINS_URL ]]; then
        ID=$(tail -1 build.log | awk '{print $3;}')
        upload2repo -t docker -i $ID -g latest -n morphio_wheel
    fi
fi
