#!/bin/bash
set -e -x

: "${UNIXY_HDF5_VERSION:=1.14.3}"
: "${CIBW_ARCHS_MACOS:=$(uname -m)}"

export INPUT=$(cd $(dirname "$1") && pwd -P)/$(basename "$1")
export OUTPUT="$INPUT/install-$CIBW_ARCHS_MACOS"


function download_unpack_hdf5 {
    pushd "$INPUT"
    local name=CMake-hdf5-$UNIXY_HDF5_VERSION.tar.gz
    if [[ ! -e $name ]]; then
        echo "Downloading & unpacking HDF5 ${UNIXY_HDF5_VERSION}"
        curl -fsSLO "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-${UNIXY_HDF5_VERSION%.*}/hdf5-$UNIXY_HDF5_VERSION/src/$name"
    fi
    tar xzf "$name"
    popd
}

if [[ "$OSTYPE" == "darwin"* ]]; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=$(nproc)
fi

INSTALL="$OUTPUT/install"

if [[ -f "$INSTALL/lib/libhdf5.a" ]]; then
    echo "using cached build"
else
    if [[ "$OSTYPE" == "darwin"* ]]; then
        export CC="/usr/bin/clang"
        export CXX="/usr/bin/clang"
        export CFLAGS="$CFLAGS -arch $CIBW_ARCHS_MACOS"
        export CPPFLAGS="$CPPFLAGS -arch $CIBW_ARCHS_MACOS"
        export CXXFLAGS="$CXXFLAGS -arch $CIBW_ARCHS_MACOS"
    fi

    echo "Building & installing hdf5"
    download_unpack_hdf5

    cmake -B "$OUTPUT/build" -G'Unix Makefiles' \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DBUILD_SHARED_LIBS=OFF \
        -DHDF5_BUILD_UTILS=OFF \
        -DHDF5_BUILD_HL_LIB=OFF \
        -DHDF5_BUILD_EXAMPLES=OFF \
        -DBUILD_TESTING=OFF \
        -DHDF5_BUILD_TOOLS=OFF \
        -DHDF5_ENABLE_SZIP_ENCODING=OFF \
        -DHDF5_ENABLE_Z_LIB_SUPPORT=OFF \
        -DCMAKE_INSTALL_PREFIX="$INSTALL" \
        -S "$INPUT/CMake-hdf5-$UNIXY_HDF5_VERSION/hdf5-$UNIXY_HDF5_VERSION"

    cmake --build "$OUTPUT/build" -j "$NPROC"
    cmake --install "$OUTPUT/build"
fi

find "$OUTPUT"
