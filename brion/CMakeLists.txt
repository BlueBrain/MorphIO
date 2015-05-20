# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

source_group(\\ FILES CMakeLists.txt)

include(files.cmake)

set(BRION_LINK_LIBRARIES ${Boost_LIBRARIES} ${HDF5_LIBRARIES}
                         ${LUNCHBOX_LIBRARIES})

common_library(Brion)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc")
  # WAR gcc 4.4 bug on BlueGene
  set_property(SOURCE mesh.cpp APPEND_STRING PROPERTY
    COMPILE_FLAGS "-Wno-uninitialized")
endif()
