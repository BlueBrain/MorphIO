set(MINIMORPH_SOURCES
    morphologyBrion.cpp
    plugin/morphologyHDF5.cpp
    plugin/morphologySWC.cpp
    )

add_library(minimorph
    ${MINIMORPH_SOURCES}
    )

set_target_properties(minimorph
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

target_link_libraries(minimorph
    PRIVATE
    HighFive
    vmmlib
    )

target_include_directories(minimorph
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

set(MORPHIO_SOURCES
    morphology.cpp
    morphologyImpl.cpp
    section.cpp
    soma.cpp
    )

add_library(morphio
    ${MORPHIO_SOURCES}
    )

set_target_properties(morphio
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

target_link_libraries(morphio
    PRIVATE
    HighFive
    vmmlib
    minimorph
    )

target_include_directories(morphio
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
