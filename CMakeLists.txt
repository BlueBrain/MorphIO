# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Brion)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "7")
set(VERSION_PATCH "0")
set(VERSION_ABI 5)

set(BRION_DESCRIPTION "The Blue Brain C++ I/O library")
set(BRION_MAINTAINER
  "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(BRION_LICENSE LGPL)

set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)
include(Common)

common_package(BBPTestData)
common_package(Boost REQUIRED COMPONENTS date_time filesystem program_options
                                         regex system unit_test_framework)
common_package(HDF5 REQUIRED COMPONENTS C CXX)
common_package(Lunchbox REQUIRED)
common_package(OpenMP)
common_package(vmmlib REQUIRED)
common_package_post()

list(APPEND BRION_DEPENDENT_LIBRARIES Boost Lunchbox vmmlib)

add_subdirectory(brion)
add_subdirectory(brain)
add_subdirectory(apps)
add_subdirectory(tests)

include(CPackConfig)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXPAND_AS_DEFINED "BRAIN_API BRION_API")
set(DOXYGEN_EXTRA_INPUT "${PROJECT_SOURCE_DIR}/README.md ${CMAKE_INSTALL_PREFIX}/include/brain")
include(DoxygenRule)
