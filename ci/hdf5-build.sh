#!/bin/bash
set -e -x


export INPUT=$(cd $(dirname "$1") && pwd -P)/$(basename "$1")
export OUTPUT="$INPUT/install-$CIBW_ARCHS_MACOS"

: "${ZLIB_VERSION:=1.3}"
: "${UNIXY_AEC_VERSION:=1.0.4}"
: "${UNIXY_HDF5_VERSION:=1.14.2}"

function download_unpack_zlib {
    pushd "$INPUT"
    rm -rf "zlib-$ZLIB_VERSION"
    echo "Downloading & unpacking ZLIB ${ZLIB_VERSION}"
    curl -sLO https://zlib.net/fossils/zlib-$ZLIB_VERSION.tar.gz
    tar xzf zlib-$ZLIB_VERSION.tar.gz
    popd
}

function download_unpack_libaec {
    pushd "$INPUT"
    rm -rf "libaec-$UNIXY_AEC_VERSION"
    echo "Downloading & unpacking aec ${UNIXY_AEC_VERSION}"
    curl -fsSLO "https://gitlab.dkrz.de/k202009/libaec/uploads/ea0b7d197a950b0c110da8dfdecbb71f/libaec-${UNIXY_AEC_VERSION}.tar.gz"
    tar zxf "libaec-$UNIXY_AEC_VERSION.tar.gz"
    popd
}

function download_unpack_hdf5 {
    pushd "$INPUT"
    rm -rf hdf5-$UNIXY_HDF5_VERSION
    echo "Downloading & unpacking HDF5 ${UNIXY_HDF5_VERSION}"
    curl -fsSLO "https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-${UNIXY_HDF5_VERSION%.*}/hdf5-$UNIXY_HDF5_VERSION/src/hdf5-$UNIXY_HDF5_VERSION.tar.gz"
    tar xzf "hdf5-$UNIXY_HDF5_VERSION.tar.gz"
    popd
}

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

        download_unpack_zlib

        pushd "$INPUT/zlib-$ZLIB_VERSION"
        ./configure                             \
            --archs="-arch $CIBW_ARCHS_MACOS"   \
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
    download_unpack_libaec
    pushd "$INPUT/libaec-$UNIXY_AEC_VERSION"
    ./configure            \
        --prefix="$OUTPUT" \
        ${HOST_ARG}        \
        --enable-static=no

    make -j "$NPROC"
    make install
    popd

    echo "Building & installing hdf5"
    download_unpack_hdf5
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
