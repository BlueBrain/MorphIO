set(cymod_name morphotool)
set(cymod_src ${cymod_name}.pyx)
set(cymod_cpp ${CMAKE_CURRENT_BINARY_DIR}/${cymod_name}.cpp)
file(GLOB_RECURSE cy_srcs *.pyx *.pxd *.pxi)

if(ENABLE_MESHER_CGAL)
    set(mesher_lib morpho_mesher)
    set(CYTHON_FLAGS "-X ENABLE_MESHER_GCAL=1")
endif()

# .pyx -> .cpp
add_custom_command(
    OUTPUT ${cymod_cpp}
    COMMAND PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} ${CYTHON_EXECUTABLE} ${CYTHON_FLAGS} --fast-fail --cplus --output-file ${cymod_cpp} ${cymod_src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${cy_srcs} morpho ${mesher_lib}
    COMMENT "Cythonizing ${cymod_src}"
 )

# .cpp -> .so
python_add_module(${cymod_name} ${cymod_cpp})
target_compile_options(${cymod_name} PRIVATE -fno-strict-aliasing)

get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
target_include_directories(${cymod_name} PUBLIC ${PYTHON_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})

target_link_libraries(${cymod_name} morpho ${mesher_lib})

