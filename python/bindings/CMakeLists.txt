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
    # No dependencies, except headers
    # DEPENDS ${cy_srcs} ${mesher_lib} morpho
    COMMENT "Cythonizing ${cymod_src}"
 )

# # .cpp -> .so
# python_add_module(${cymod_name} ${cymod_cpp})
# target_compile_options(${cymod_name} PRIVATE -fno-strict-aliasing)
# add_dependencies(${cymod_name} pure_py)
# target_link_libraries(${cymod_name} morpho ${mesher_lib})
#
# get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
# target_include_directories(${cymod_name} PUBLIC ${PYTHON_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})
