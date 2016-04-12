# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Juan Hernando <jhernando@fi.upm.es>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

source_group(\\ FILES CMakeLists.txt)

set(BRAIN_PUBLIC_HEADERS
  circuit.h
  types.h
  neuron/morphology.h
  neuron/section.h
  neuron/soma.h
  neuron/types.h
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
set(BRAIN_LINK_LIBRARIES PUBLIC Brion vmmlib)
set(BRAIN_INCLUDE_NAME brain)
set(BRAIN_NAMESPACE brain)

if(TARGET MVDTool)
  list(APPEND BRAIN_LINK_LIBRARIES PRIVATE MVDTool)
endif()
common_library(Brain)
if(TARGET MVDTool)
  add_definitions(-DBRAIN_USE_MVD3)
endif()
