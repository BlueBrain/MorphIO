cmake_minimum_required(VERSION 3.2)
project(MorphIO VERSION 2.0.0)

add_subdirectory(3rdparty/coding-conventions/cpp)
bob_begin_cxx_flags()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(BUILD_BINDINGS "Build the python bindings" ON)

add_subdirectory(3rdparty)
add_subdirectory(src)

if(BUILD_BINDINGS)
  add_subdirectory(binds/python)
endif(BUILD_BINDINGS)
# add_subdirectory(apps)

install(
    DIRECTORY include/morphio
    DESTINATION include
    )

install(
  EXPORT MorphIOTargets
  NAMESPACE MorphIO::
  DESTINATION lib/cmake/MorphIO
  )

install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/CMake/MorphIOConfig.cmake
  DESTINATION lib/cmake/MorphIO
  )
