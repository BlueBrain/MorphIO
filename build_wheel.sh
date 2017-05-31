#!/usr/bin/env bash
cmake ..                                                            \
    -DMAKE_BUILD_TYPE=Release                                       \
    -DUSE_PYTHON_VERSION=2                                          \
    -DPYTHON_EXECUTABLE:FILEPATH=/opt/python/cp27-cp27mu/bin/python \
    -DPYTHON_INCLUDE_DIR=/opt/python/cp27-cp27mu/include/python2.7/ \
    -DBOOST_INCLUDEDIR=/usr/local/include/                          \
    -DCOMMON_LIBRARY_TYPE=STATIC                                    \
    -DOpenMP_CXX_FLAGS=''                                           \
    -DOpenMP_C_FLAGS=''                                             \
    -DHDF5_USE_STATIC_LIBRARIES=on
