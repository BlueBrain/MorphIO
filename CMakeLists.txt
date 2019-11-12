cmake_minimum_required(VERSION 3.2)
project(MorphIO VERSION 2.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(BUILD_BINDINGS "Build the python bindings" ON)
option(${PROJECT_NAME}_CXX_WARNINGS "Compile C++ with warnings" ON)

include(${PROJECT_SOURCE_DIR}/CMake/CompilerFlags.cmake)
set(CMAKE_CXX_FLAGS "${FLAGS}")

add_subdirectory(3rdparty)
add_subdirectory(src)

if(BUILD_BINDINGS)
  add_subdirectory(binds/python)
endif(BUILD_BINDINGS)

set(MORPHIO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
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
