cmake_minimum_required(VERSION 3.2)
project(MorphIO VERSION 3.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)


add_subdirectory(3rdparty)
add_subdirectory(src)

add_subdirectory(binds/python)
add_subdirectory(apps)

#add_subdirectory(tests)
#include(CPackConfig)

set(CMAKE_INSTALL_PREFIX /tmp)

install(
    DIRECTORY include/morphio
    DESTINATION include
    )
