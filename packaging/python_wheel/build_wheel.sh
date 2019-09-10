#!/usr/bin/env bash
set -euxo pipefail

MORPHIO_BASE=$(git rev-parse --show-toplevel)
WHEELHOUSE=$MORPHIO_BASE/packaging/python_wheel/wheelhouse/

UNAME=$(uname)
if [[ $UNAME == Darwin ]]; then
    PYTHON_VERSIONS="cp27-cp27m"
    AUDIT_CMD="delocate-wheel -w $WHEELHOUSE"
else
    PYTHON_VERSIONS="cp27-cp27mu cp27-cp27m cp35-cp35m cp36-cp36m cp37-cp37m"
    AUDIT_CMD="auditwheel repair -w $WHEELHOUSE"
fi

build_morphio()
{
    local version=$1
    if [[ $UNAME == Darwin ]]; then
        local PYTHON=$(which python)
    else
        # in manylinux1 docker image
        local PYTHON=/opt/python/$version/bin/python
    fi

    cd "$MORPHIO_BASE"
    $PYTHON setup.py bdist_wheel
    $AUDIT_CMD $MORPHIO_BASE/dist/*${version}*
    rm -rf build dist morphio.egg-info
}

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done
