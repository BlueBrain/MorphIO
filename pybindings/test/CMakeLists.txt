
add_test(NAME "Morphotool_working" 
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
         COMMAND python test_mtool.py)

add_test(NAME "Morphology_layer_working" 
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
         COMMAND python test_morphology.py)

