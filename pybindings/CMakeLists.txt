
set(cymod_name morphotool)



set(cymod_src ${cymod_name}.pyx)
set(cymod_cpp ${CMAKE_CURRENT_BINARY_DIR}/${cymod_name}.cpp)

# .pyx -> .cpp
add_custom_command(
    OUTPUT ${cymod_cpp}
    COMMAND ${CYTHON_EXECUTABLE} --cplus --output-file ${cymod_cpp} ${cymod_src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cythonizing ${cymod_src}"
 )

# .cpp -> .so
python_add_module(${cymod_name} ${cymod_cpp})

get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
target_include_directories(${cymod_name} PUBLIC ${PYTHON_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})

target_link_libraries(${cymod_name}  ${CMAKE_THREAD_LIBS_INIT}
                               ${HDF5_C_LIBRARIES}  
                               ${Boost_FILESYSTEM_LIBRARIES}
)
#set_target_properties(cysteps PROPERTIES 
#    LIBRARY_OUTPUT_DIRECTORY ${steps_package_dir}
#    OUTPUT_NAME ${cymodname}
# )

