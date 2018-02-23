#!/usr/bin/env bash
set -e

PYTHON_VERSIONS="cp27-cp27mu cp27-cp27m cp35-cp35m cp36-cp36m"
NUMPY_VERSION=1.12.0
PACKAGING_DIR=/io/packaging/python_wheel/
WHEELHOUSE=/io/packaging/python_wheel/wheelhouse/


get_python_include()
{
    local PYTHON=$1; shift
    $PYTHON -c 'import distutils.sysconfig as s; print(s.get_python_inc())'
}

build_boost_python()
{
    local PYTHON=$1; shift
    local PYTHON_INC=$1; shift

    pushd /boost_1_59_0

    ./b2 --clean
    ./bootstrap.sh --with-libraries=python --with-python=$PYTHON

    ./b2 -j2 -q                 \
        --build-type=minimal    \
        cxxflags=-fPIC          \
        cflags=-fPIC            \
        threading=multi         \
        link=static             \
        include=$PYTHON_INC     \
        install
     popd
}

build_morphio()
{
    local PYTHON=/opt/python/$version/bin/python
    $PYTHON setup.py bdist_wheel --bdist-dir=/home
    auditwheel repair -w $WHEELHOUSE dist/*${version}*
    popd

    cd ..
    rm -rf build
}

for version in $PYTHON_VERSIONS; do
    build_morphio $version
done

chown -R $UID.$UID $WHEELHOUSE
