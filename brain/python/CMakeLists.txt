#
# Copyright (c) 2011-2015, ahmet.bilgili@epfl.ch
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

if( NOT Boost_PYTHON${USE_BOOST_PYTHON_VERSION}_LIBRARY OR
    NOT PYTHONLIBS_FOUND OR NOT PYTHON_EXECUTABLE )

  message(STATUS "No Boost.Python or Python found. Disabling python bindings")
  return()
endif()

set(BRAIN_PYTHON_SOURCE_FILES
  brain.cpp
  spikeReportWriter.cpp
  spikeReportReader.cpp
  spikes.cpp)

include_directories(${PYTHON_INCLUDE_DIRS})
add_library(brain_python MODULE ${BRAIN_PYTHON_SOURCE_FILES})
add_dependencies(brain_python Brain)

target_link_libraries(brain_python
  Brain ${PYTHON_LIBRARIES} ${Boost_PYTHON${USE_BOOST_PYTHON_VERSION}_LIBRARY})

set_target_properties(brain_python PROPERTIES
  OUTPUT_NAME "_brain" PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain)
configure_file(brain/__init__.py
               ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain/__init__.py)

set_property(GLOBAL APPEND PROPERTY BRION_ALL_DEP_TARGETS brain_python)

install(TARGETS brain_python
  LIBRARY DESTINATION ${PYTHON_LIBRARY_SUFFIX}/brain)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain/__init__.py
        DESTINATION ${PYTHON_LIBRARY_SUFFIX}/brain)
