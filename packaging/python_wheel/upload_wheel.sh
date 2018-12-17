#!/usr/bin/env bash
set -o xtrace
BASE=$(git rev-parse --show-toplevel)

if [ "$RELEASE" == "true" ]; then
    CHANNEL=dev
else
    CHANNEL=dev
fi

for wheel in `ls $BASE/packaging/python_wheel/wheelhouse/MorphIO*.whl`; do
    upload2repo -t python -r $CHANNEL -f $wheel
done

upload2repo -t python -r $CHANNEL -f $BASE/dist/MorphIO-*.tar.gz


# cleanup afterwards
docker run \
    -v $BASE:/io:Z \
    bbpdocker.epfl.ch/morphio_wheel \
    /bin/rm -rf /io/packaging/python_wheel/wheelhouse /io/dist
