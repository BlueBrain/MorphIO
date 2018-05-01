set(MORPHIO_SOURCES
  morphology.cpp
  plugin/morphologyHDF5.cpp
  plugin/morphologySWC.cpp
  plugin/morphologyASC.cpp
  mitochondria.cpp
  mito_section.cpp
  mito_iterators.cpp
  properties.cpp
  iterators.cpp
  morphology.cpp
  soma.cpp
  section.cpp
  vector_utils.cpp
  mut/iterators.cpp
  mut/section.cpp
  mut/soma.cpp
  mut/morphology.cpp
  mut/mitochondria.cpp
  mut/writers.cpp
  mut/modifiers.cpp
  )

# by default, -fPIC is only used of the dynamic library build
# This forces the flag also for the static lib
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Building object files only once. They will be used for the shared and static library
add_library(morphio_obj OBJECT ${MORPHIO_SOURCES})

target_include_directories(morphio_obj
  PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
   $<TARGET_PROPERTY:GSL,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:lexertl14,INTERFACE_INCLUDE_DIRECTORIES>
  )

set_target_properties(morphio_obj
  PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

add_library(morphio_static STATIC $<TARGET_OBJECTS:morphio_obj>)
add_library(morphio_shared SHARED $<TARGET_OBJECTS:morphio_obj>)
set_target_properties(morphio_shared PROPERTIES OUTPUT_NAME "morphio")

target_link_libraries(morphio_static PUBLIC GSL PRIVATE HighFive lexertl14)
target_link_libraries(morphio_shared PUBLIC GSL PRIVATE HighFive lexertl14)

install(
  # DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  TARGETS morphio_shared
  DESTINATION lib
  )
