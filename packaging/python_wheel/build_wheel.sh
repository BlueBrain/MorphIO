#!/usr/bin/env bash
# set -e
set -o xtrace

export MORPHIO_BASE=$(readlink -f "$( cd "$(dirname "$0")" ; pwd -P )"/../..)
export WHEELHOUSE=${MORPHIO_BASE}/packaging/python_wheel/wheelhouse/

if [ "$(uname)" == "Darwin" ]; then
    export PYTHON_VERSIONS="cp27-cp27m"
    export AUDIT_CMD="delocate-wheel -w $WHEELHOUSE"
else
    # in manylinux1 docker image
    export CXX=/toolchain/bin/c++
    export LD_LIBRARY_PATH=/toolchain/lib:${LD_LIBRARY_PATH}
    export AUDIT_CMD="auditwheel repair -w $WHEELHOUSE"
    export PYTHON_VERSIONS="cp35-cp35m cp36-cp36m cp27-cp27mu cp27-cp27m"
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
    ls -alR build || true
    rm -rf build dist morphio.egg-info bin
    ls -alR build || true

    $PYTHON setup.py bdist_wheel
    ls -lrt build/bdist.linux-x86_64/wheel
    echo "((((((((After build))))))))"
    git status
    ${AUDIT_CMD} ${MORPHIO_BASE}/dist/*${version}*
    rm -rf build dist morphio.egg-info bin

    echo "((((((((After rm))))))))"
    git status
}

rm ${MORPHIO_BASE}/.gitignore

rm -rf ${MORPHIO_BASE}/bin

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done
