# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          ahmet.bilgili@epfl.ch
#                          juan.hernando@epfl.ch
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

if(NOT Boost_PYTHON${USE_BOOST_PYTHON_VERSION}_LIBRARY OR
   NOT PYTHONLIBS_FOUND OR NOT PYTHON_EXECUTABLE OR
   NOT NUMPY_FOUND)

  message(STATUS "Not configured brain.py: Boost.Python, Python or Numpy not found")
  return()
endif()

include(PythonDocstrings)

set(BRAIN_PYTHON_SOURCES
  brain.cpp
  circuit.cpp
  compartmentReport.cpp
  spikeReportReader.cpp
  synapses.cpp
  neuron/morphology.cpp
)

docstrings(BRAIN_PYTHON_SOURCES BRAIN_PUBLIC_HEADERS
  ${PROJECT_BINARY_DIR}/include)

list(APPEND BRAIN_PYTHON_SOURCES
  arrayHelpers.cpp
  helpers.cpp
  spikeReportWriter.cpp
  spikes.cpp
  submodules.cpp
  test.cpp
  neuron/module.cpp
  ${PROJECT_BINARY_DIR}/docstrings/cpp/docstrings.cpp
)

include_directories(${PROJECT_BINARY_DIR}/docstrings/cpp
                    ${PYTHON_INCLUDE_DIRS}
                    ${PYTHON_NUMPY_INCLUDE_DIR})

add_library(brain_python MODULE ${BRAIN_PYTHON_SOURCES})
common_compile_options(brain_python)
if(TARGET ${PROJECT_NAME}-docstrings)
  add_dependencies(brain_python Brain ${PROJECT_NAME}-docstrings)
  target_compile_definitions(brain_python PRIVATE BRAIN_HAS_DOCSTRINGS)
endif()

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

add_dependencies(Brion-all brain_python)

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
