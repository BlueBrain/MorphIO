set(cymod_src "morphotool.pyx")
file(GLOB_RECURSE cy_srcs *.pyx *.pxd *.pxi)

if(ENABLE_MESHER_CGAL)
    set(mesher_lib morpho_mesher)
    set(CYTHON_FLAGS "-X ENABLE_MESHER_GCAL=1")
endif()

# .pyx -> .cpp
add_custom_command(
    OUTPUT ${CYBINDINGS_CPP}
    COMMAND PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} ${CYTHON_EXECUTABLE} ${CYTHON_FLAGS} --fast-fail --cplus --output-file ${CYBINDINGS_CPP} ${cymod_src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # No dependencies, except own sources
    DEPENDS ${cy_srcs}
    COMMENT "Cythonizing ${cymod_src}"
 )
