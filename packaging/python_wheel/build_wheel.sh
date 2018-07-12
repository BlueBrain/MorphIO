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
    # cp35-cp35m is ignored while https://github.com/pybind/pybind11/issues/314
    # is not solved
    export PYTHON_VERSIONS="cp36-cp36m cp27-cp27mu cp27-cp27m"
fi

rm -rf ${MORPHIO_BASE}/envs/
build_morphio()
{
    if [ "$(uname)" == "Darwin" ]; then
        local PYTHON=$(which python)
    else
        # in manylinux1 docker image
        local PYTHON=/opt/python/$version/bin/python
    fi

    cd ${MORPHIO_BASE}
    rm -rf build dist morphio.egg-info bin

    env_name=${MORPHIO_BASE}/envs/env-${version}
    $(dirname $PYTHON)/pyvenv ${env_name}
    source ${env_name}/bin/activate
    which pip
    pip install --upgrade pip
    pip install cython==0.28
    # $PYTHON setup.py bdist_wheel
    "${PYBIN}/pip" wheel ${MORPHIO_BASE} -w ${WHEELHOUSE}
    ls -lrt build/bdist.linux-x86_64/wheel
    git status
    ${AUDIT_CMD} ${MORPHIO_BASE}/dist/*${version}*
    rm -rf build dist morphio.egg-info bin
    git status
    deactivate
}

rm -rf ${MORPHIO_BASE}/bin

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done
