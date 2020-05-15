set(TESTS_SRC
    main.cpp
    test_morphology.cpp
)

add_executable(unittests ${TESTS_SRC})

set_target_properties(unittests
  PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

target_link_libraries(unittests
    PRIVATE morphio_static HighFive
)

add_test(NAME unittests
         COMMAND unittests
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         )
