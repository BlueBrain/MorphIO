set(MORPHIO_SOURCES
    morphologyBrion.cpp
    plugin/morphologyHDF5.cpp
    plugin/morphologySWC.cpp
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
    glm
    )

target_include_directories(morphio
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ../3rdparty/glm
    )
