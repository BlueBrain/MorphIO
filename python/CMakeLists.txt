if(BUILD_PYTHON_MODULE)

# Even though we dont reqire the lib, we need headers
find_package(PythonInterpLibs)

option(REBUILD_BINDINGS "Rebuild Python bindings from Cython sources" OFF)
set(CYBINDINGS_CPP "${CMAKE_SOURCE_DIR}/python/morphotool/morphotool.cpp" CACHE STRING "Location of the cpp source of the bindings")

if(REBULD_BINDINGS OR (NOT EXISTS ${CYBINDINGS_CPP}))
	find_package(Cython 0.25.0 REQUIRED)
	message(STATUS "${Cyan}Will rebuild Cython bindings!${ColorReset}")
else()
	message(STATUS "${Cyan}Reusing bindings existing cpp!${ColorReset}")
endif()

get_target_property(dirs morpho INCLUDE_DIRECTORIES)

add_custom_target(pymorpho ALL
    COMMAND ${PYTHON_EXECUTABLE} setup.py build_ext "-I$<JOIN:${dirs},:>" -L${CMAKE_BINARY_DIR}/src/morpho  install --install-lib=${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CYBINDINGS_CPP} morpho
	)


add_subdirectory(bindings)

if(MORPHOTOOL_UNIT_TESTS)
	find_package(NumPy REQUIRED) # required for unit test execution
	add_subdirectory(test)
endif()


## INSTALL

set(PY_INSTALL_OPTIONS "--prefix=${CMAKE_INSTALL_PREFIX}")
install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py install ${PY_INSTALL_OPTIONS} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")

endif()
