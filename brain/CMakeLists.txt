# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Juan Hernando <jhernando@fi.upm.es>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

source_group(\\ FILES CMakeLists.txt)

## force inclue directory for cmake 2 compat
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../MVDTool/include ${CMAKE_CURRENT_SOURCE_DIR}/../MVDTool/deps/highfive/include)

set(BRAIN_PUBLIC_HEADERS
  circuit.h
  types.h
  neuron/morphology.h
  neuron/section.h
  neuron/soma.h
  )

set(BRAIN_HEADERS
  neuron/morphologyImpl.h
  )

set(BRAIN_SOURCES
  circuit.cpp
  neuron/morphology.cpp
  neuron/morphologyImpl.cpp
  neuron/section.cpp
  neuron/soma.cpp
  )

set(BRAIN_PUBLIC_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
set(BRAIN_LINK_LIBRARIES PUBLIC Brion vmmlib
                         PRIVATE ${HDF5_LIBRARIES})
set(BRAIN_INCLUDE_NAME brain)
set(BRAIN_NAMESPACE brain)

common_library(Brain)
