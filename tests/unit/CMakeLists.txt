
if(UNIT_TESTS)

find_package(Boost 1.41 QUIET REQUIRED COMPONENTS filesystem system unit_test_framework)


SET(MVD2_TEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../circuit.mvd2)
SET(MVD3_TEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../circuit.mvd3)


add_definitions(-DMVD2_FILENAME="${MVD2_TEST_FILE}")
add_definitions(-DMVD3_FILENAME="${MVD3_TEST_FILE}")
if(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_COMPILER_IS_CLANG)
  add_definitions(-Wno-unused-local-typedefs)
endif()

include_directories(${HDF5_INCLUDE_DIRS})


## mvd2

file(GLOB  tests_mvd2_src "*mvd2.cpp")
add_executable(test_mvd2 ${tests_mvd2_src})

target_link_libraries(test_mvd2 ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

add_test(NAME test_parser_mvd2 COMMAND test_mvd2)


# mvd3
file(GLOB  tests_mvd3_src "*mvd3.cpp")
add_executable(test_mvd3 ${tests_mvd3_src})

target_link_libraries(test_mvd3 ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${HDF5_LIBRARIES})

add_test(NAME test_parser_mvd3 COMMAND test_mvd3)

endif()
