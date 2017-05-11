
add_test(NAME Morphotool_working
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
         COMMAND python test_mtool.py)

# setup pythonpath manually to allow test execution
# with a simple ctest 
set_property(TEST Morphotool_working PROPERTY 
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../:$ENV{PYTHONPATH}"
)



add_test(NAME Morphology_layer_working
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
         COMMAND python test_morphology.py)

set_property(TEST Morphology_layer_working PROPERTY 
    ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../:$ENV{PYTHONPATH}"
)
