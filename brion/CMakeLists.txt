# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(BRION_PUBLIC_HEADERS
  blueConfig.h
  circuit.h
  compartmentReport.h
  compartmentReportPlugin.h
  enums.h
  mesh.h
  morphology.h
  morphologyPlugin.h
  pluginInitData.h
  spikeReport.h
  spikeReportPlugin.h
  synapseSummary.h
  synapse.h
  target.h
  types.h
  )

set(BRION_HEADERS
  constants.h
  detail/mesh.h
  detail/meshBinary.h
  detail/meshHDF5.h
  detail/lockHDF5.h
  detail/silenceHDF5.h
  detail/utilsHDF5.h
  detail/skipWhiteSpace.h
  )

set(BRION_SOURCES
  blueConfig.cpp
  circuit.cpp
  compartmentReport.cpp
  mesh.cpp
  morphology.cpp
  spikeReport.cpp
  synapseSummary.cpp
  synapse.cpp
  target.cpp
  )

if(MSVC)
  add_definitions(/wd4251 # missing dll-interface for H5::Exception
    )
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
  PUBLIC Lunchbox Servus vmmlib
  PRIVATE ${_plugin_lib} ${HDF5_LIBRARIES}
          ${Boost_FILESYSTEM_LIBRARIES} ${Boost_REGEX_LIBRARIES}
          ${CMAKE_THREADS_LIB_INIT}
)

common_library(Brion)
