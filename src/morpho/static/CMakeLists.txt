# Build static library in sub folder
# Easy linking to the static library when intended

add_library(morphostatic STATIC $<TARGET_OBJECTS:morpho-obj>)
set_target_properties(morphostatic PROPERTIES
                        OUTPUT_NAME morpho
                    )
