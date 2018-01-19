# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Juan Hernando <jhernando@fi.upm.es>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(BRAIN_PUBLIC_HEADERS
  morphology.h
  section.h
  soma.h
  types.h
  types.h
  )

set(BRAIN_HEADERS
  morphologyImpl.h
  )

set(BRAIN_SOURCES
  morphology.cpp
  morphologyImpl.cpp
  section.cpp
  soma.cpp
  )

set(BRAIN_LINK_LIBRARIES
  PUBLIC vmmlib ${HDF5_C_LIBRARIES}
  PRIVATE HighFive
)

FILE(GLOB_RECURSE BRAIN_SOURCES ${CMAKE_SOURCE_DIR}/brain/*.cpp)

add_library(brain
    SHARED
    ${BRAIN_PUBLIC_HEADERS} ${BRAIN_HEADERS} ${BRAIN_SOURCES}
    )
target_link_libraries(brain ${BRAIN_LINK_LIBRARIES})

include_directories(brain
    ${CMAKE_SOURCE_DIR}
)
