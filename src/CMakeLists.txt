set(MORPHIO_SOURCES
    collection.cpp
    dendritic_spine.cpp
    endoplasmic_reticulum.cpp
    enums.cpp
    errorMessages.cpp
    glial_cell.cpp
    mito_section.cpp
    mitochondria.cpp
    morphology.cpp
    morphology.cpp
    mut/dendritic_spine.cpp
    mut/endoplasmic_reticulum.cpp
    mut/glial_cell.cpp
    mut/mito_section.cpp
    mut/mitochondria.cpp
    mut/modifiers.cpp
    mut/morphology.cpp
    mut/section.cpp
    mut/soma.cpp
    mut/writer_asc.cpp
    mut/writer_hdf5.cpp
    mut/writer_swc.cpp
    mut/writer_utils.cpp
    point_utils.cpp
    properties.cpp
    readers/morphologyASC.cpp
    readers/morphologyHDF5.cpp
    readers/morphologySWC.cpp
    readers/vasculatureHDF5.cpp
    section.cpp
    shared_utils.cpp
    soma.cpp
    vasc/properties.cpp
    vasc/section.cpp
    vasc/vasculature.cpp
    version.cpp
    )

if(NOT MORPHIO_VERSION_STRING)
    execute_process(COMMAND git describe --tags
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE GIT_VERSION_FAILED
                    OUTPUT_VARIABLE MORPHIO_VERSION_STRING
                    ERROR_VARIABLE GIT_VERSION_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
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
  )

target_include_directories(morphio_obj
  SYSTEM
  PUBLIC
   $<TARGET_PROPERTY:gsl-lite,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:lexertl,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE
   $<TARGET_PROPERTY:ghc_filesystem,INTERFACE_INCLUDE_DIRECTORIES>
  )

set_target_properties(morphio_obj
  PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

if (MORPHIO_ENABLE_COVERAGE)
  include(CodeCoverage)
  append_coverage_compiler_flags_to_target(morphio_obj)
endif()


if (NOT WIN32)
  target_compile_options(morphio_obj
    PRIVATE
    -Wall
    -Wextra
    -Wnon-virtual-dtor
    -pedantic
    )
endif()

add_library(morphio_static STATIC $<TARGET_OBJECTS:morphio_obj>)
add_library(morphio_shared SHARED $<TARGET_OBJECTS:morphio_obj>)

set_target_properties(morphio_shared PROPERTIES OUTPUT_NAME "morphio"
                                                EXPORT_NAME "morphio"
                                                SOVERSION "0.0.0")
foreach(TARGET morphio_shared morphio_static)
  target_include_directories(${TARGET}
    SYSTEM
    PUBLIC
     $<BUILD_INTERFACE:${MORPHIO_INCLUDE_DIR}>
     $<INSTALL_INTERFACE:include>
     $<TARGET_PROPERTY:gsl-lite,INTERFACE_INCLUDE_DIRECTORIES>
     $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
     $<TARGET_PROPERTY:lexertl,INTERFACE_INCLUDE_DIRECTORIES>
     )
  target_link_libraries(${TARGET} PUBLIC gsl-lite PRIVATE HighFive lexertl)
endforeach(TARGET)

install(
  # DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  TARGETS morphio_shared
  EXPORT MorphIOTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION include
)
