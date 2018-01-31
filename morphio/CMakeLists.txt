set(MORPHIO_SOURCES
    morphology.cpp
    morphologyImpl.cpp
    section.cpp
    soma.cpp
    )

add_library(morphio SHARED
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
    )
