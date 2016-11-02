# Copyright (c) 2016, EPFL/Blue Brain Project
#                     Juan Hernando Vieites <juan.hernando@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

if(NOT SPHINX_FOUND)
  return()
endif()

configure_file(conf.py.in ${PROJECT_BINARY_DIR}/doc/python/conf.py)

file(COPY index.rst DESTINATION ${PROJECT_BINARY_DIR}/doc/python)

add_custom_target(${PROJECT_NAME}-python-doc-html
  ${SPHINX_EXECUTABLE} -b html . ${PROJECT_BINARY_DIR}/doc/html/python
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc/python
  DEPENDS index.rst conf.py.in
  COMMENT "Generating Python documentation using Sphinx" VERBATIM)
set_target_properties(${PROJECT_NAME}-python-doc-html PROPERTIES
  EXCLUDE_FROM_DEFAULT_BUILD ON FOLDER "doxygen")

add_dependencies(${PROJECT_NAME}-python-doc-html brain_python)
add_dependencies(${PROJECT_NAME}-doxygen ${PROJECT_NAME}-python-doc-html)
