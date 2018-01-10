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
  pluginInitData.h
  types.h
  )

set(BRION_HEADERS
  constants.h
  detail/lockHDF5.h
  detail/morphologyHDF5.h
  detail/skipWhiteSpace.h
  detail/utilsHDF5.h
  )

set(BRION_SOURCES
  morphology.cpp
  )

if(MSVC)
  add_definitions(/wd4251) # missing dll-interface for H5::Exception
endif()

add_subdirectory(plugin)

set(_plugin_lib BrionPlugins)

# when linking Brion statically, only visible and used symbols from an
# application or shared library will be linked. Meaning the plugin registerers
# for spike- and compartmentreports are skipped. With whole-archive, we include
# all symbols. As it serves as a switch, we have to reset it for any subsequent
# library.
if(COMMON_LIBRARY_TYPE MATCHES "STATIC" AND NOT MSVC)
  set(_plugin_lib -Wl,--whole-archive BrionPlugins -Wl,--no-whole-archive)
endif()

set(BRION_PUBLIC_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
set(BRION_LINK_LIBRARIES
  PUBLIC Servus vmmlib
  PRIVATE Lunchbox HighFive ${_plugin_lib} ${Boost_FILESYSTEM_LIBRARIES}
          ${Boost_REGEX_LIBRARIES} ${CMAKE_THREADS_LIB_INIT}
)

common_library(Brion)
