cmake_minimum_required(VERSION 3.2)
project(MorphIO VERSION 2.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(BUILD_BINDINGS "Build the python bindings" ON)
option(MorphIO_CXX_WARNINGS "Compile C++ with warnings" ON)
option(EXTERNAL_CATCH2 "Use Catch2 from external source" OFF)
option(EXTERNAL_HIGHFIVE "Use HighFive from external source" OFF)
option(EXTERNAL_PYBIND11 "Use pybind11 from external source" OFF)
option(MORPHIO_TESTS "Build tests" ON)
option(MORPHIO_USE_DOUBLE "Use doubles instead of floats" OFF)

set(MORPHIO_ENABLE_COVERAGE OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(MORPHIO_ENABLE_COVERAGE ON)
endif()

if(MORPHIO_USE_DOUBLE)
  add_definitions(-DMORPHIO_USE_DOUBLE)
  message("Floating Point Type: double")
else()
  message("Floating Point Type: float")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
include(CompilerFlags)
set(CMAKE_CXX_FLAGS "${FLAGS}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (EXTERNAL_HIGHFIVE)
  find_package(HighFive REQUIRED)
endif()
if (EXTERNAL_PYBIND11)
  find_package(pybind11 REQUIRED CONFIG)
endif()
set(MORPHIO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/)

add_subdirectory(3rdparty)
add_subdirectory(src)

if(BUILD_BINDINGS)
  add_subdirectory(binds/python)
endif(BUILD_BINDINGS)

install(
  DIRECTORY ${MORPHIO_INCLUDE_DIR}
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


if (MORPHIO_TESTS)
  enable_testing()
  if (EXTERNAL_CATCH2)
    find_package(Catch2 REQUIRED)
  else()
    add_subdirectory(3rdparty/Catch2)
    include(3rdparty/Catch2/contrib/Catch.cmake)
  endif()
  add_subdirectory(tests)
endif()
