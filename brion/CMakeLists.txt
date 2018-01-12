# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(BRION_PUBLIC_HEADERS
  enums.h
  morphology.h
  morphologyPlugin.h
  morphologyPlugin.ipp
  types.h
  )

set(BRION_HEADERS
  detail/lockHDF5.h
  detail/morphologyHDF5.h
  detail/skipWhiteSpace.h
  detail/utilsHDF5.h
  )

set(BRION_SOURCES
  morphology.cpp
  )

set(BRION_LINK_LIBRARIES
  PUBLIC vmmlib ${HDF5_C_LIBRARIES}
  PRIVATE HighFive
)

FILE(GLOB_RECURSE BRION_SOURCES ${CMAKE_SOURCE_DIR}/brion/*.cpp)

add_library(brion
    SHARED
    ${BRION_PUBLIC_HEADERS} ${BRION_HEADERS} ${BRION_SOURCES}
    )
target_link_libraries(brion ${BRION_LINK_LIBRARIES})

include_directories(brion
    ${HDF5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
)
