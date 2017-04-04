

list(APPEND SRC_SPATIAL "morpho_spatial.cpp" "gmsh_exporter.cpp"  "x3d_exporter.cpp" "morpho_tree.cpp" "morpho_stats.cpp")



add_library(morpho SHARED ${SRC_SPATIAL} ${MORPHO_HEADERS})

target_link_libraries(morpho
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${HDF5_C_LIBRARIES}  ${Boost_FILESYSTEM_LIBRARIES}
                             )


install(TARGETS morpho DESTINATION ${LIB_INSTALL_DIR})

