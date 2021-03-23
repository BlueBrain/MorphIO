set(TESTS_SRC
        main.cpp
        test_morphology_readers.cpp
        test_immutable_morphology.cpp
        test_mutable_morphology.cpp
        )
set(TESTS_LINK_LIBRAIRIES morphio_static HighFive)

add_executable(unittests ${TESTS_SRC})

set_target_properties(unittests
  PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

if (MORPHIO_ENABLE_COVERAGE)
  include(CodeCoverage)
  set(COVERAGE_LCOV_EXCLUDES '/usr/*' '${PROJECT_SOURCE_DIR}/include/*' '${PROJECT_SOURCE_DIR}/3rdparty/*')
  SETUP_TARGET_FOR_COVERAGE_LCOV(
      NAME coverage
      EXECUTABLE ctest
      DEPENDENCIES unittests
  )
  list(APPEND TESTS_LINK_LIBRAIRIES gcov)
endif()

target_link_libraries(unittests
    PRIVATE ${TESTS_LINK_LIBRAIRIES}
)

add_test(NAME unittests
         COMMAND unittests
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         )
