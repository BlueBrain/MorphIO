

list(APPEND SRC_SPATIAL "morpho_reader_h5v1.cpp"
                        "morpho_circuit.cpp"
                        "morpho_reader.cpp"
                        "morpho_swc.cpp")



add_library(morpho_io SHARED ${SRC_SPATIAL} ${MORPHO_HEADERS})

target_link_libraries(morpho_io
                            morpho
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${HDF5_C_LIBRARIES}
                            ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES}
                             )


install(TARGETS morpho_io DESTINATION ${LIB_INSTALL_DIR})

