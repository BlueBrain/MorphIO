#!/bin/bash
set -x -e

OUTPUT=$1

mkdir -p "$OUTPUT"
cd "$OUTPUT"

echo "Downloading & unpacking aec ${UNIXY_AEC_VERSION}"
curl -fsSLO "https://gitlab.dkrz.de/k202009/libaec/uploads/ea0b7d197a950b0c110da8dfdecbb71f/libaec-${UNIXY_AEC_VERSION}.tar.gz"
tar zxf "libaec-$UNIXY_AEC_VERSION.tar.gz"

echo "Downloading & unpacking HDF5 ${UNIXY_HDF5_VERSION}"
curl -fsSLO "https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-${UNIXY_HDF5_VERSION%.*}/hdf5-$UNIXY_HDF5_VERSION/src/hdf5-$UNIXY_HDF5_VERSION.tar.gz"
tar xzf "hdf5-$UNIXY_HDF5_VERSION.tar.gz"
