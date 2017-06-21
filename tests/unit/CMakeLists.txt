
if(MORPHOTOOL_UNIT_TESTS)


find_package(Boost 1.41.0 REQUIRED system unit_test_framework)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )

link_directories( ${Boost_LIBRARY_DIRS} )

if(NOT Boost_USE_STATIC_LIBS)
        add_definitions(-DBOOST_TEST_DYN_LINK=TRUE)
endif()

# add info about test file
add_definitions( -DH5V1_MORPHO_TEST_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/dend-C090905B_axon-sm110125a1-3_idA.h5\" )
add_definitions( -DSWC_MORPHO_TEST_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10-2012-02-09-001.CNG.swc\" )

## morpho tool export
file(GLOB  tests_morpho_src "*morpho*.cpp")
add_executable(tests_morpho_bin ${tests_morpho_src})

target_link_libraries(tests_morpho_bin ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} morpho morpho_io)

add_test(NAME test_morpho COMMAND tests_morpho_bin)


## gmsh tests
file(GLOB  tests_gmsh_src "*gmsh*.cpp")
add_executable(tests_gmsh_bin ${tests_gmsh_src}  )

target_link_libraries(tests_gmsh_bin ${HDF5_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} morpho morpho_io)

add_test(NAME test_gmsh COMMAND tests_gmsh_bin)


## h5v1 tests
file(GLOB  tests_h5v1_src "*h5v1*.cpp")
add_executable(tests_h5v1_bin ${tests_h5v1_src}  )

target_link_libraries(tests_h5v1_bin ${HDF5_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} morpho morpho_io)

add_test(NAME test_h5v1 COMMAND tests_h5v1_bin)

## SWC tests
add_executable(tests_swc_bin test_swc.cpp)

target_link_libraries(tests_swc_bin ${HDF5_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} morpho morpho_io)
target_compile_options(tests_swc_bin PUBLIC "-DMORPHO_OUTPUT_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"")
add_test(NAME test_swc COMMAND tests_swc_bin)

endif()
