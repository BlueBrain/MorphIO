# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          ahmet.bilgili@epfl.ch
#                          juan.hernando@epfl.ch
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

if( NOT Boost_PYTHON${USE_BOOST_PYTHON_VERSION}_LIBRARY OR
    NOT PYTHONLIBS_FOUND OR NOT PYTHON_EXECUTABLE )

  message(STATUS "No Boost.Python or Python found. Disabling python bindings")
  return()
endif()

set(BRAIN_PYTHON_SOURCE_FILES
  arrayHelpers.cpp
  brain.cpp
  circuit.cpp
  spikeReportWriter.cpp
  spikeReportReader.cpp
  spikes.cpp
  synapses.cpp
  submodules.cpp
  test.cpp
  neuron/module.cpp
  neuron/morphology.cpp
)
include_directories(${PYTHON_INCLUDE_DIRS}
                    ${PYTHON_NUMPY_INCLUDE_DIR})

add_library(brain_python MODULE ${BRAIN_PYTHON_SOURCE_FILES})
common_compile_options(brain_python)
add_dependencies(brain_python Brain)

target_link_libraries(brain_python
  PRIVATE Brain ${PYTHON_LIBRARIES}
          ${Boost_PYTHON${USE_BOOST_PYTHON_VERSION}_LIBRARY})
if(BBPTESTDATA_FOUND)
  target_link_libraries(brain_python PRIVATE BBPTestData)
endif()

set_target_properties(brain_python PROPERTIES
  OUTPUT_NAME "_brain" PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain)
configure_file(brain/__init__.py
               ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain/__init__.py)

set_property(GLOBAL APPEND PROPERTY BRION_ALL_DEP_TARGETS brain_python)

macro(COPY_AND_INSTALL_PACKAGE_DIRECTORY DIRECTORY)
    file(COPY brain/${DIRECTORY}
      DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain)

    install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain/${DIRECTORY}
      DESTINATION ${PYTHON_LIBRARY_SUFFIX}/brain
      COMPONENT COMMON)
endmacro()

install(TARGETS brain_python
  LIBRARY DESTINATION ${PYTHON_LIBRARY_SUFFIX}/brain)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/brain/__init__.py
        DESTINATION ${PYTHON_LIBRARY_SUFFIX}/brain)
copy_and_install_package_directory(neuron)
