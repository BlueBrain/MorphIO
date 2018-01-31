# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#
cmake_minimum_required(VERSION 3.2)
project(MorphIO VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(3rdparty)

find_package(HDF5 REQUIRED)

add_subdirectory(minimorph)
add_subdirectory(morphio)

add_subdirectory(binds/python)

#add_subdirectory(tests)
#include(CPackConfig)
