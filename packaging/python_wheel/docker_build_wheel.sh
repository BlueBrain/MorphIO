#!/usr/bin/env bash
set -e

PYTHON_VERSIONS="cp27-cp27mu cp27-cp27m cp35-cp35m cp36-cp36m"
NUMPY_VERSION=1.12.0
PACKAGING_DIR=/io/packaging/python_wheel/
WHEELHOUSE=/io/packaging/python_wheel/wheelhouse/
export CXX=/io/packaging/python_wheel/toolchain/bin/c++
# export CC=/io/packaging/python_wheel/toolchain/bin/gcc
export LD_LIBRARY_PATH=/io/packaging/python_wheel/toolchain/lib:${LD_LIBRARY_PATH}


get_python_include()
{
    local PYTHON=$1; shift
    $PYTHON -c 'import distutils.sysconfig as s; print(s.get_python_inc())'
}

build_morphio()
{
    local PYTHON=/opt/python/$version/bin/python
    cd /io
    rm -rf build dist morphio.egg-info
    $PYTHON setup.py bdist_wheel --bdist-dir=/home
    auditwheel repair -w $WHEELHOUSE dist/*${version}*
    rm -rf build dist
}

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done

chown -R $UID.$UID $WHEELHOUSE
