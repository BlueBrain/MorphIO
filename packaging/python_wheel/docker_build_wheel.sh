#!/usr/bin/env bash
set -e

PYTHON_VERSIONS="cp27-cp27mu cp35-cp35m cp36-cp36m"
NUMPY_VERSION=1.12.0
PACKAGING_DIR=/io/packaging/python_wheel/
WHEELHOUSE=/io/packaging/python_wheel/wheelhouse/
EXT_SRC=/io/ext_src


get_python_include()
{
    local PYTHON=$1; shift
    $PYTHON -c 'import distutils.sysconfig as s; print(s.get_python_inc())'
}

install_reqs()
{
    yum install -y zlib-devel bzip2-devel
}

# Need to build this from source, since the binaries from Kitware don't
# run with the old version of the libraries in this Centos version
build_cmake()
{
    [[ -e /usr/local/bin/cmake ]] && return

    [[ -e /tmp/cmake-3.7.2 ]] || tar xfz $EXT_SRC/cmake-3.7.2.tar.gz -C /tmp
    cd /tmp/cmake-3.7.2
    ./configure
    make -j8 install
}

# h5 morphologies depend on hdf5, so need to build it
build_hdf()
{
    [[ -e /usr/local/lib/libhdf5.a ]] && return

    [[ -e /tmp/hdf5-1.8.17 ]] || tar xjf $EXT_SRC/hdf5-1.8.17.tar.bz2 -C /tmp
    cd /tmp/hdf5-1.8.17
    ./configure CFLAGS=-fPIC CXXFLAGS=-fPIC   \
        --prefix=/usr/local                   \
        --enable-cxx                          \
        --with-pic                            \
        --disable-shared                      \
        --with-zlib=/usr/include/,/usr/lib64/
    make -j8 install

    # --disable-shared doesn't add libz to the archive, so it
    # must be done manually at the end, there aren't any collisions in
    # the .a file, so currently safe
    ar x /usr/lib64/libz.a
    ar r /usr/local/lib/libhdf5.a *.o
}

# Needed for boost_python/boost_regex/boost_filesystem
build_boost()
{
    local PYTHON=$1; shift
    local PYTHON_INC=$1; shift
    tar xfz $EXT_SRC/boost_1_59_0.tar.gz -C /tmp
    cd /tmp/boost_1_59_0

    rm -f project-config.jam
    ./bootstrap.sh --with-python=$PYTHON
    ./b2 clean

    # HACK: b2 doesn't take into account the python ABIFLAGS,
    # so it doesn't find the correct header files
    perl -i -pe "s#(using python [^;]+);#\1 : $PYTHON_INC ;#" project-config.jam

    ./b2 -j8 -q --without-mpi            \
        cxxflags=-fPIC                   \
        cflags=-fPIC                     \
        threading=multi                  \
        link=static                      \
        install
}

build_brain()
{
    local version=$1; shift

    local PYTHON=/opt/python/$version/bin/python
    local MAJOR_VERSION=$($PYTHON -c 'import sys; print(sys.version_info[0])')
    local PYTHON_INC=$(get_python_include $PYTHON)

    # for each python version the ABI may have changed, so rebuild
    # the boost python bindings
    build_boost $PYTHON $PYTHON_INC

    /opt/python/$version/bin/pip install "numpy==$NUMPY_VERSION"

    cd /io/build

    # disabling OMP so we don't have to link in libgomp
    cmake ..                                                            \
        -DCMAKE_BUILD_TYPE=Release                                      \
        -DUSE_PYTHON_VERSION=$MAJOR_VERSION                             \
        -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON                            \
        -DPYTHON_INCLUDE_DIR=$PYTHON_INC                                \
        -DBOOST_INCLUDEDIR=/usr/local/include/                          \
        -DCOMMON_LIBRARY_TYPE=STATIC                                    \
        -DOpenMP_CXX_FLAGS=''                                           \
        -DOpenMP_C_FLAGS=''                                             \
        -DHDF5_USE_STATIC_LIBRARIES=ON
    make -j8 brain_python

    cp $PACKAGING_DIR/setup.py $PACKAGING_DIR/setup.cfg lib
    pushd lib
    $PYTHON setup.py bdist_wheel
    auditwheel repair -w $WHEELHOUSE dist/*${version}*
    popd
}

install_reqs
build_cmake
build_hdf

for version in $PYTHON_VERSIONS; do
    build_brain $version
done
