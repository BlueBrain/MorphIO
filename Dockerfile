FROM quay.io/pypa/manylinux1_x86_64
MAINTAINER BlueBrain NSE(NeuroScientific Engineering)

# Downloads
#wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.17/src/hdf5-1.8.17.tar.bz2
#wget http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.gz
#wget https://cmake.org/files/v3.7/cmake-3.7.2.tar.gz

RUN yum install -y zlib-devel bzip2-devel

# Need to build this from source, since the binaries available don't run
# on the old version
ADD ext_src/cmake-3.7.2.tar.gz /tmp/
RUN cd /tmp/cmake-3.7.2 \
    && ./configure \
    && make -j8 \
    && make install

# h5 morphologies depend on hdf5
ADD ext_src/hdf5-1.8.17.tar.bz2 /tmp/
RUN cd /tmp/hdf5-1.8.17 \
    && ./configure CFLAGS=-fPIC CXXFLAGS=-fPIC \
        --prefix=/usr/local \
        --enable-cxx \
        --with-pic \
        --disable-shared \
    && make -j8 \
    && make install

# Need: boost_python/boost_regex/boost_filesystem
ADD ext_src/boost_1_59_0.tar.gz /tmp/
RUN cd /tmp/boost_1_59_0 \
    && ./bootstrap.sh --with-python=/opt/python/cp27-cp27mu/bin/python \
    && ./b2 -j8 -q --without-mpi cxxflags=-fPIC cflags=-fPIC threading=multi link=static install 

RUN /opt/python/cp27-cp27mu/bin/pip install 'numpy==1.10'
