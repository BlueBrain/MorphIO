#!/usr/bin/env bash
BASE=$(git rev-parse --show-toplevel)

for wheel in `ls $BASE/packaging/python_wheel/wheelhouse/brain*.whl`; do
    upload2repo -t python -r dev -f $wheel
done

# cleanup afterwards
docker run --rm \
    -v $BASE:/io:Z \
    bbpdocker.epfl.ch/brain_wheel \
    /bin/rm -rf /io/packaging/python_wheel/wheelhouse
