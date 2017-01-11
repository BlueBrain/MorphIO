


list(APPEND SRC_EXPORTER "mesh_exporter.cpp" "morpho_tool.cpp" "morpho_spatial.cpp" "x3d_exporter.cpp" )

list(APPEND SRC_MESHER "cgal_mesher.cpp" )

include_directories(${CFAL_INCLUDE_DIRS})

add_definitions(${CFAL_DEFINITIONS})

add_executable(morpho-tool ${SRC_EXPORTER} ${SRC_MESHER} ${MORPHO_HEADERS_BITS} ${MORPHO_HEADERS})
target_link_libraries(morpho-tool
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${HDF5_C_LIBRARIES}  ${Boost_FILESYSTEM_LIBRARIES}
                             ${Boost_SYSTEM_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                            ${CGAL_LIBRARIES}
                            ## additional dependencies required for CGAL
                            ${Boost_THREAD_LIBRARIES} gmp mpfr
                             )





install(TARGETS morpho-tool RUNTIME DESTINATION ${BIN_INSTALL_DIR})
