cmake_minimum_required(VERSION 3.2)
project(MorphIO VERSION 3.0.0)

option(BUILD_SHARED_LIBS "Build the static library" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)


add_subdirectory(3rdparty)

add_subdirectory(minimorph)
add_subdirectory(morphio)

add_subdirectory(binds/python)

#add_subdirectory(tests)
#include(CPackConfig)
