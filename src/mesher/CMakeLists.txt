
if(ENABLE_MESHER_CGAL)

find_package(CGAL REQUIRED)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)




list(APPEND SRC_MESHER "cgal_mesher.cpp" )

include_directories(${CFAL_INCLUDE_DIRS} ${GMP_INCLUDE_DIRS} ${MPFR_INCLUDE_DIRS})

add_definitions(${CFAL_DEFINITIONS})

add_library(morpho_mesher SHARED ${SRC_MESHER} ${MORPHO_HEADERS})
target_link_libraries(morpho_mesher
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${Boost_FILESYSTEM_LIBRARIES}
                            ${Boost_SYSTEM_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                            ${CGAL_LIBRARIES} ${GMP_LIBRARIES} ${MPFR_LIBRARIES}
                            ## additional dependencies required for CGAL
                            ${Boost_THREAD_LIBRARIES} ${GMP_LIBRARIES} ${MPFR_LIBRARIES}

                            morpho
                             )





install(TARGETS morpho_mesher DESTINATION ${LIB_INSTALL_DIR})



endif()
