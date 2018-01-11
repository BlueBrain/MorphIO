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

#set(_plugin_lib BrionPlugins)
#
#set(BRION_PUBLIC_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})

set(BRION_LINK_LIBRARIES
  PUBLIC Servus vmmlib
  PRIVATE Lunchbox HighFive ${_plugin_lib} ${Boost_FILESYSTEM_LIBRARIES}
          ${Boost_REGEX_LIBRARIES} ${CMAKE_THREADS_LIB_INIT}
)

#common_library(Brion)

add_library(brion
    ${BRION_PUBLIC_HEADERS} ${BRION_HEADERS} ${BRION_SOURCES}
    )
target_link_libraries(brion ${BRION_LINK_LIBRARIES})

include_directories(brion
    ${CMAKE_SOURCE_DIR}
)

add_subdirectory(plugin)

