set(MORPHIOCPP_SOURCES
  morphology.cpp
  plugin/morphologyHDF5.cpp
  plugin/morphologySWC.cpp
  plugin/morphologyASC.cpp
  properties.cpp
  iterators.cpp
  morphology.cpp
  soma.cpp
  section.cpp
  vector_utils
  sectionBuilder.cpp
  )

add_library(morphiocpp
  ${MORPHIOCPP_SOURCES}
  )

set_target_properties(morphiocpp
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

target_link_libraries(morphiocpp
  PUBLIC
  GSL

  PRIVATE
  HighFive
  lexertl14
  )

target_include_directories(morphiocpp
  PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  )
