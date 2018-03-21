#!/usr/bin/env bash
set -e

MORPHIO_BASE=$(realpath "$( cd "$(dirname "$0")" ; pwd -P )"/../..)
WHEELHOUSE=${MORPHIO_BASE}/packaging/python_wheel/wheelhouse/
export LD_LIBRARY_PATH=${MORPHIO_BASE}/packaging/python_wheel/toolchain/lib:${LD_LIBRARY_PATH}

if [ "$(uname)" == "Darwin" ]; then
    PYTHON_VERSIONS="cp27-cp27m"
    AUDIT_CMD=delocate-wheel
else
    export CXX=${MORPHIO_BASE}/packaging/python_wheel/toolchain/bin/c++
    # export CC=${MORPHIO_BASE}/packaging/python_wheel/toolchain/bin/gcc
    AUDIT_CMD="auditwheel repair -w $WHEELHOUSE"
    PYTHON_VERSIONS="cp27-cp27mu cp27-cp27m cp35-cp35m cp36-cp36m"
fi


build_morphio()
{
    echo $(uname)
    if [ "$(uname)" == "Darwin" ]; then
        local PYTHON=$(which python)
    else
        local PYTHON=/opt/python/$version/bin/python
    fi
    cd ${MORPHIO_BASE}
    rm -rf build dist morphio.egg-info
    $PYTHON setup.py bdist_wheel --bdist-dir=${MORPHIO_BASE}/dist
    ${AUDIT_CMD} ${MORPHIO_BASE}/dist/*${version}*
    # rm -rf build dist
}

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done

# chown -R $UID.$UID $WHEELHOUSE
