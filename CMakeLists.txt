# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

cmake_minimum_required(VERSION 3.2)
project(morphio VERSION 3.0.0)
#set(Brion_VERSION_ABI 9)

#set(BRION_DESCRIPTION "The Blue Brain C++ I/O library")
#set(BRION_MAINTAINER_NAME "Blue Brain Project")
#set(BRION_MAINTAINER_EMAIL "<bbp-open-source@googlegroups.com>")
#set(BRION_MAINTAINER "${BRION_MAINTAINER_NAME} ${BRION_MAINTAINER_EMAIL}")
#set(BRION_LICENSE LGPL)
#set(BRION_DEB_DEPENDS libhdf5-serial-dev libboost-date-time-dev
#  libboost-filesystem-dev libboost-iostreams-dev libboost-regex-dev
#  libboost-system-dev libboost-test-dev libboost-thread-dev)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(3rdparty)

find_package(Boost REQUIRED COMPONENTS
  date_time filesystem iostreams program_options regex system unit_test_framework)
#find_package(HighFive REQUIRED)
find_package(HDF5 QUIET REQUIRED)

#common_find_package(Lunchbox REQUIRED)
#common_find_package(Boost COMPONENTS python${USE_BOOST_PYTHON_VERSION})
#common_find_package(NumPy 1.7)
#common_find_package(OpenMP)
#common_find_package(PythonInterp)
#common_find_package(PythonLibs)
#common_find_package(Servus REQUIRED)
#common_find_package(Sphinx 1.3)
#common_find_package_post()

#list(APPEND BRION_DEPENDENT_LIBRARIES Boost vmmlib)

add_subdirectory(brion)
#add_subdirectory(tests)

#include(CPackConfig)
