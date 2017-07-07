
add_test(NAME Python_Morphotool_working
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND python test_mtool.py)

add_test(NAME Python_Morphology_layer_working
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND python test_morphology.py)

add_test(NAME Python_Unit_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND python unit.py -v)


# setup pythonpath manually to allow test execution with a simple ctest
set_property(TEST Python_Morphotool_working PROPERTY
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../:$ENV{PYTHONPATH}"
)

set_property(TEST Python_Morphology_layer_working PROPERTY
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../:$ENV{PYTHONPATH}"
)

set_property(TEST Python_Unit_tests PROPERTY
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../:$ENV{PYTHONPATH}"
)
