#!/usr/bin/env bash
set -e

export MORPHIO_BASE=$(realpath "$( cd "$(dirname "$0")" ; pwd -P )"/../..)
export WHEELHOUSE=${MORPHIO_BASE}/packaging/python_wheel/wheelhouse/

if [ "$(uname)" == "Darwin" ]; then
    export PYTHON_VERSIONS="cp27-cp27m"
    export AUDIT_CMD="delocate-wheel -w $WHEELHOUSE"
else
    # in manylinux1 docker image
    export CXX=${MORPHIO_BASE}/packaging/python_wheel/toolchain/bin/c++
    export AUDIT_CMD="auditwheel repair -w $WHEELHOUSE"
    export PYTHON_VERSIONS="cp27-cp27mu cp27-cp27m cp35-cp35m cp36-cp36m"
    export LD_LIBRARY_PATH=${MORPHIO_BASE}/packaging/python_wheel/toolchain/lib:${LD_LIBRARY_PATH}
fi


build_morphio()
{
    if [ "$(uname)" == "Darwin" ]; then
        local PYTHON=$(which python)
    else
        # in manylinux1 docker image
        local PYTHON=/opt/python/$version/bin/python
    fi
    cd ${MORPHIO_BASE}
    rm -rf build dist morphio.egg-info
    $PYTHON setup.py bdist_wheel
    ${AUDIT_CMD} ${MORPHIO_BASE}/dist/*${version}*
    rm -rf build dist
}

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done

# chown -R $UID.$UID $WHEELHOUSE
