# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#
# Change this number when adding tests to force a CMake run: 1

if(NOT BBPTESTDATA_FOUND)
  if(COMMON_ENABLE_COVERAGE)
    message(FATAL_ERROR "Coverage report generation requires test data")
  endif()
  if(NOT TARGET ${PROJECT_NAME}-tests)
    add_custom_target(${PROJECT_NAME}-tests)
    set_target_properties(${PROJECT_NAME}-tests PROPERTIES
      EXCLUDE_FROM_DEFAULT_BUILD ON FOLDER ${PROJECT_NAME}/tests)
  endif()
  return()
endif()

configure_file(paths.h.in ${PROJECT_BINARY_DIR}/tests/paths.h)
include_directories(${PROJECT_BINARY_DIR}/tests)

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} Brain Brion BBPTestData)
include(CommonCTest)
