set(MORPHIO_SOURCES
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

add_library(morphio
    ${MORPHIO_SOURCES}
    )

set_target_properties(morphio
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

target_link_libraries(morphio
    PRIVATE
    HighFive
    GSL
    )

target_include_directories(morphio
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/lexertl14
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/GSL/include
    )
