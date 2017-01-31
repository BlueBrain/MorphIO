# Copyright (c) 2016, EPFL/Blue Brain Project
#                     Juan Hernando <juan.hernando@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#
# Change this number when adding tests to force a CMake run: 2

if(NOT TARGET BBPTestData OR NOT TARGET brain_python)
  return()
endif()

set(PYTHON_TEST_DEPENDENCIES brain_python)
include(CommonPythonCTest)
configure_file(setup.py.in ${PYTHON_TEST_OUTPUT_PATH}/setup.py)
