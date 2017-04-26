set(cymod_name morphotool)
set(cymod_src ${cymod_name}.pyx)
set(cymod_cpp ${CMAKE_CURRENT_BINARY_DIR}/${cymod_name}.cpp)
file(GLOB_RECURSE cy_srcs *.pyx *.pxd *.pxi)
file(GLOB py_srcs *.py)

# Pure python
add_custom_target(pure_py)
foreach(pyfile ${py_srcs})
    add_custom_command(TARGET pure_py PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} ${CMAKE_CURRENT_BINARY_DIR}/
                       COMMENT "Copying ${pyfile}"
                       )
endforeach()

if(ENABLE_MESHER_CGAL)
    set(mesher_lib morpho_mesher)
    set(CYTHON_FLAGS "-X ENABLE_MESHER_GCAL=1")
endif()

# .pyx -> .cpp
add_custom_command(
    OUTPUT ${cymod_cpp}
    COMMAND PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} ${CYTHON_EXECUTABLE} ${CYTHON_FLAGS} --fast-fail --cplus --output-file ${cymod_cpp} ${cymod_src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${cy_srcs} ${mesher_lib} morpho
    COMMENT "Cythonizing ${cymod_src}"
 )

# .cpp -> .so
python_add_module(${cymod_name} ${cymod_cpp})
target_compile_options(${cymod_name} PRIVATE -fno-strict-aliasing)
add_dependencies(${cymod_name} pure_py)
target_link_libraries(${cymod_name} morpho ${mesher_lib})

get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
target_include_directories(${cymod_name} PUBLIC ${PYTHON_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})

