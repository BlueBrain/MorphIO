# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(BRIONPLUGINS_HEADERS
  morphologyHDF5.h
  morphologySWC.h
)

set(BRIONPLUGINS_SOURCES
  morphologyHDF5.cpp
  morphologySWC.cpp
)

set(BRIONPLUGINS_LINK_LIBRARIES
  PRIVATE Lunchbox Keyv Servus vmmlib HighFive ${Boost_FILESYSTEM_LIBRARIES}
          ${Boost_REGEX_LIBRARIES} ${Boost_IOSTREAMS_LIBRARIES}
          ${CMAKE_THREADS_LIB_INIT}
)

if(TARGET ZeroEQ)
  list(APPEND BRIONPLUGINS_HEADERS morphologyZeroEQ.h)
  list(APPEND BRIONPLUGINS_SOURCES morphologyZeroEQ.cpp)
  list(APPEND BRIONPLUGINS_LINK_LIBRARIES ZeroEQ)
endif()

set(BRIONPLUGINS_OMIT_LIBRARY_HEADER ON)
set(BRIONPLUGINS_OMIT_VERSION_HEADERS ON)
common_library(BrionPlugins)
