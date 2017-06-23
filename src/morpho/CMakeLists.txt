list(APPEND SRC_SPATIAL "morpho_spatial.cpp"
                 "gmsh_exporter.cpp"
		 "x3d_exporter.cpp"
                 "morpho_tree.cpp"
                 "morpho_tree_algorithm.cpp"
                 "morpho_stats.cpp"
		 "morpho_transform.cpp")

add_library(morpho-obj OBJECT ${SRC_SPATIAL} )
set_property(TARGET morpho-obj PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(morpho SHARED $<TARGET_OBJECTS:morpho-obj>)

target_link_libraries(morpho
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${Boost_FILESYSTEM_LIBRARIES}
                            ${Boost_SYSTEM_LIBRARIES}

                             )


install(TARGETS morpho DESTINATION ${LIB_INSTALL_DIR})
