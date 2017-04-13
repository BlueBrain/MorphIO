
set(cymod_name morphotool)
set(cymod_src ${cymod_name}.pyx)
set(cymod_cpp ${CMAKE_CURRENT_BINARY_DIR}/${cymod_name}.cpp)
file(GLOB cy_srcs *.pyx *.pxd *.pxi)

# Set and clean optional_mods
set(optional_mods_file ${CMAKE_CURRENT_SOURCE_DIR}/optional_mods.pxi)
file(REMOVE ${optional_mods_file})
file(WRITE ${optional_mods_file} "")

if(ENABLE_MESHER_CGAL)
    set(mesher_lib morpho_mesher)
    file(APPEND ${optional_mods_file} "include 'morpho_mesher.pxi'")
endif()

# .pyx -> .cpp
add_custom_command(
    OUTPUT ${cymod_cpp}
    COMMAND ${CYTHON_EXECUTABLE} --cplus --output-file ${cymod_cpp} ${cymod_src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${cy_srcs} morpho ${mesher_lib}
    COMMENT "Cythonizing ${cymod_src}"
 )

# .cpp -> .so
python_add_module(${cymod_name} ${cymod_cpp})
target_compile_options(${cymod_name} PRIVATE -fno-strict-aliasing)

get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
target_include_directories(${cymod_name} PUBLIC ${PYTHON_INCLUDE_DIR} ${INCLUDE_DIRECTORIES})

#target_link_libraries( ${cymod_name}  
#                       ${CMAKE_THREAD_LIBS_INIT}
#                       ${HDF5_C_LIBRARIES}  
#                       ${Boost_FILESYSTEM_LIBRARIES} 
#                      )

target_link_libraries(${cymod_name} morpho ${mesher_lib})

