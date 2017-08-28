if(CYTHON_FOUND AND PYTHON_NUMPY_FOUND)
    message(STATUS "${Cyan}Will cythonize bindings when modified.${ColorReset}")
    
    set(cymod_src "morphotool.pyx")
    file(GLOB_RECURSE cy_srcs *.pyx *.pxd *.pxi)

    # .pyx -> .cpp
    add_custom_command(
        OUTPUT ${cybindings_cpp}
        COMMAND PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} ${CYTHON_EXECUTABLE} ${CYTHON_FLAGS} -I includes --fast-fail --cplus --output-file  ${cybindings_cpp} ${cymod_src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        # No dependencies, except own sources
        DEPENDS ${cymod_src} ${cy_srcs}
        COMMENT "Cythonizing ${cymod_src}"
     )
endif()

add_custom_target(cybindings DEPENDS ${cybindings_cpp})

