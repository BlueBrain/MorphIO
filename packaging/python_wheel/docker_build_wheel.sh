#!/usr/bin/env bash
set -e

PYTHON_VERSIONS="cp27-cp27mu cp35-cp35m cp36-cp36m"
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

build_brain()
{
    local version=$1; shift

    local PYTHON=/opt/python/$version/bin/python
    local MAJOR_VERSION=$($PYTHON -c 'import sys; print(sys.version_info[0])')
    local PYTHON_INC=$(get_python_include $PYTHON)

    # for each python version the ABI may have changed, so rebuild
    # the boost python bindings
    build_boost_python $PYTHON $PYTHON_INC

    /opt/python/$version/bin/pip install "numpy==$NUMPY_VERSION"
    /opt/python/$version/bin/pip install "sphinx==1.3.6" lxml

    mkdir -p /io/build
    cd /io/build
    rm -rf *

    cmake ..                                                            \
        -DCMAKE_BUILD_TYPE=Release                                      \
        -DCLONE_SUBPROJECTS=ON                                          \
        -DSUBPROJECT_ZeroEQ=OFF                                         \
        -DUSE_PYTHON_VERSION=$MAJOR_VERSION                             \
        -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON                            \
        -DPYTHON_INCLUDE_DIR=$PYTHON_INC                                \
        -DBOOST_INCLUDEDIR=/usr/local/include/                          \
        -DCOMMON_LIBRARY_TYPE=STATIC                                    \
        -DHDF5_USE_STATIC_LIBRARIES=ON                                  \
        -DSPHINX_ROOT=/opt/python/$version
    make -j2 brain_python

    cp packaging/python_wheel/setup.py $PACKAGING_DIR/setup.cfg lib
    pushd lib
    $PYTHON setup.py bdist_wheel --bdist-dir=/home
    auditwheel repair -w $WHEELHOUSE dist/*${version}*
    popd

    cd ..
    rm -rf build
}

for version in $PYTHON_VERSIONS; do
    build_brain $version
done

chown -R $UID.$UID $WHEELHOUSE
