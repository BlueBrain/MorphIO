FROM quay.io/pypa/manylinux1_x86_64:2478ba64a9951d7d31bd76c16c9ad747922e3a5abe3b82ca9a23efce86d7a19d
RUN yum install -y zlib-devel bzip2-devel flex

############
# CMake
############

# Need to build this from source, since the binaries from Kitware don't
# run with the old version of the libraries in this Centos version
RUN curl -OL https://cmake.org/files/v3.7/cmake-3.7.2.tar.gz && tar xfz cmake-3.7.2.tar.gz && cd cmake-3.7.2 && ./configure && make -j2 install
RUN rm -rf cmake-3.7.2.tar.gz cmake-3.7.2


############
# HDF5
############

RUN curl -OL https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8/hdf5-1.8.17/src/hdf5-1.8.17.tar.bz2 && tar xfj hdf5-1.8.17.tar.bz2
RUN cd hdf5-1.8.17 && ./configure CFLAGS=-fPIC CXXFLAGS=-fPIC --prefix=/usr/local --with-pic --disable-shared --with-zlib=/usr/include/,/usr/lib64/ && make -j2 install

# --disable-shared doesn't add libz to the archive, so it
# must be done manually at the end, there aren't any collisions in
# the .a file, so currently safe
RUN cd hdf5-1.8.17 && ar x /usr/lib64/libz.a && ar r /usr/local/lib/libhdf5.a *.o
RUN rm -rf hdf5-1.8.17.tar.bz2 hdf5-1.8.17

############
# GCC 7.1
# Taken from this discussion about a recent GCC in manylinux.
# https://github.com/pypa/manylinux/issues/118#issuecomment-317238069
#
# In case this URL disappears. There is a copy of the tarball here:
# /nfs4/bbp.epfl.ch/user/bcoste/public/gcc-7.1-binutils-2.28-centos5-x86-64.tar.bz2
############
RUN curl -OL https://github.com/Noctem/pogeo-toolchain/releases/download/v1.3/gcc-7.1-binutils-2.28-centos5-x86-64.tar.bz2 && tar -xjvf gcc-7.1-binutils-2.28-centos5-x86-64.tar.bz2
