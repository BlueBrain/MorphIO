# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Brion VERSION 1.9.0)
set(Brion_VERSION_ABI 7)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)

set(BRION_DESCRIPTION "The Blue Brain C++ I/O library")
set(BRION_MAINTAINER
  "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(BRION_LICENSE LGPL)
set(BRION_DEB_DEPENDS libhdf5-serial-dev libboost-date-time-dev
  libboost-filesystem-dev libboost-regex-dev libboost-system-dev
  libboost-test-dev libboost-thread-dev)

set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)
include(Common)

common_find_package(BBPTestData)
common_find_package(Boost REQUIRED COMPONENTS
  date_time filesystem program_options regex system thread unit_test_framework)
common_find_package(Boost COMPONENTS python${USE_BOOST_PYTHON_VERSION})
common_find_package(HDF5 SYSTEM REQUIRED COMPONENTS C CXX)
common_find_package(Lunchbox REQUIRED)
common_find_package(MVDTool)
common_find_package(NumPy 1.7)
common_find_package(OpenMP)
common_find_package(PythonInterp)
common_find_package(PythonLibs)
common_find_package(Servus REQUIRED)
common_find_package(vmmlib REQUIRED)
common_find_package_post()

list(APPEND BRION_DEPENDENT_LIBRARIES Boost Lunchbox Servus vmmlib)

add_subdirectory(brion)
if(NOT COMMON_USE_CXX03)
  add_subdirectory(brain)
endif()
add_subdirectory(apps)
add_subdirectory(tests)

include(CPackConfig)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXPAND_AS_DEFINED "BRAIN_API BRION_API")
set(DOXYGEN_EXTRA_INPUT "${PROJECT_SOURCE_DIR}/README.md ${CMAKE_INSTALL_PREFIX}/include/brain")
set(DOXYGEN_PREDEFINED_MACROS "BRAIN_API:=")
include(DoxygenRule)
