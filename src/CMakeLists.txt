

add_subdirectory(tool)
add_subdirectory(morpho)
add_subdirectory(io)



# ================================================================
# Build static morphoio library (including morpho)
#
add_library(morphostatic STATIC $<TARGET_OBJECTS:morpho-obj> $<TARGET_OBJECTS:morphoio-obj>)
set_target_properties(morphostatic
                      PROPERTIES
                      OUTPUT_NAME morpho_io)
