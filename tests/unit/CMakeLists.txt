
if(MORPHOTOOL_UNIT_TESTS)


find_package(Boost 1.41.0 REQUIRED system unit_test_framework)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )

link_directories( ${Boost_LIBRARY_DIRS} )

if(NOT Boost_USE_STATIC_LIBS)
        add_definitions(-DBOOST_TEST_DYN_LINK=TRUE)
endif()



file(GLOB  tests_morpho_src "*morpho*.cpp")
add_executable(tests_morpho_bin ${tests_morpho_src})

target_link_libraries(tests_morpho_bin ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

add_test(NAME test_morpho COMMAND tests_morpho_bin)



endif()
