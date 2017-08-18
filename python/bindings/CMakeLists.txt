
set(cymod_src "morphotool.pyx")
file(GLOB_RECURSE cy_srcs *.pyx *.pxd *.pxi)

if(REBUILD_PYTHON_BINDINGS OR (NOT EXISTS ${CYBINDINGS_CPP}))
    if(NOT CYTHON_FOUND)
        message(FATAL_ERROR "Cython is required to rebuild python bindings but was not found" )
    endif()
    if(NOT PYTHON_NUMPY_FOUND)
        message(FATAL_ERROR "NumPy is required to rebuild bindings ")
    endif()
    message(STATUS "${Cyan}Will rebuild Cython bindings!${ColorReset}")
    file(REMOVE ${CYBINDINGS_CPP})
endif()


# .pyx -> .cpp
add_custom_command(
    OUTPUT ${CYBINDINGS_CPP}
    COMMAND PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} ${CYTHON_EXECUTABLE} ${CYTHON_FLAGS} -I includes --fast-fail --cplus --output-file  ${CYBINDINGS_CPP} ${cymod_src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # No dependencies, except own sources
    DEPENDS ${cymod_src}
    COMMENT "Cythonizing ${cymod_src}"
 )
add_custom_target(cybindings_cpp DEPENDS ${CYBINDINGS_CPP})
