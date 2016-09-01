# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#
# Change this number when adding tests to force a CMake run: 1

if(NOT BBPTESTDATA_FOUND)
  if(COMMON_ENABLE_COVERAGE)
    message(FATAL_ERROR "Coverage report generation requires test data")
  endif()
  return()
endif()

configure_file(paths.h.in ${PROJECT_BINARY_DIR}/tests/paths.h)
include_directories(${PROJECT_BINARY_DIR}/tests)

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} Brion BBPTestData)
if(NOT COMMON_USE_CXX03)
  list(APPEND TEST_LIBRARIES Brain)
else()
  file(GLOB EXCLUDE_FROM_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} brain/*.cpp)
endif()
include(CommonCTest)

add_subdirectory(brain/python)