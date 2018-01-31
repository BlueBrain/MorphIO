set(MINIMORPH_SOURCES
    morphology.cpp
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
    )
