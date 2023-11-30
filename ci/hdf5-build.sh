#!/bin/bash
set -e -x

export INPUT=$(cd $(dirname "$1") && pwd -P)/$(basename "$1")
export OUTPUT="$INPUT/install-$CIBW_ARCHS_MACOS"

if [[ "$OSTYPE" == "darwin"* ]]; then
    lib_name=libhdf5.dylib
    NPROC=$(sysctl -n hw.ncpu)
else
    lib_name=libhdf5.so
    NPROC=$(nproc)
fi

if [[ -f "$OUTPUT/lib/$lib_name" ]]; then
    echo "using cached build"
else
    if [[ "$OSTYPE" == "darwin"* ]]; then
        export CC="/usr/bin/clang"
        export CXX="/usr/bin/clang"
        export CFLAGS="$CFLAGS -arch $CIBW_ARCHS_MACOS"
        export CPPFLAGS="$CPPFLAGS -arch $CIBW_ARCHS_MACOS"
        export CXXFLAGS="$CXXFLAGS -arch $CIBW_ARCHS_MACOS"

        ZLIB_VERSION="1.3"

        pushd "$INPUT"
        curl -sLO https://zlib.net/fossils/zlib-$ZLIB_VERSION.tar.gz
        tar xzf zlib-$ZLIB_VERSION.tar.gz
        cd zlib-$ZLIB_VERSION
        ./configure \
            --prefix="$OUTPUT"
        make
        make install
        popd

        ZLIB_ARG="--with-zlib=$OUTPUT"
        if [[ "$CIBW_ARCHS_MACOS" = "arm64" ]]; then
            HOST_ARG="--host=aarch64-apple-darwin"
        fi
    fi

    echo "Building & installing libaec"
    pushd "$INPUT/libaec-$UNIXY_AEC_VERSION"
    ./configure            \
        --prefix="$OUTPUT" \
        ${HOST_ARG}        \
        --enable-static=no

    make -j "$NPROC"
    make install
    popd

    pushd "$INPUT/hdf5-$UNIXY_HDF5_VERSION"
    ./configure                        \
        --prefix="$OUTPUT"             \
        --with-szlib="$OUTPUT"         \
        ${HOST_ARG}                    \
        ${ZLIB_ARG}                    \
        --enable-build-mode=production \
        --enable-tools=no              \
        --enable-tests=no              \
        --enable-static=no

    # {
    # work around for crosscompile getting: `./H5detect: Bad CPU type in executable`
    # so build it on the host architecture instead
    pushd src
    ( export ARCHFLAGS= ; make H5detect H5make_libsettings CFLAGS= CCLDFLAGS= CXXFLAGS= CPPFLAGS= )
    popd
    # }

    make -j "$NPROC"
    make install
    popd

    file "$OUTPUT"/lib/*
fi
