if(BUILD_PYTHON_MODULE)

find_package(PythonInterpLibs)
find_package(PythonLibs) # python_add_module support
find_package(Cython 0.25.0 REQUIRED )


if(MORPHOTOOL_UNIT_TESTS)
	find_package(NumPy REQUIRED) # required for unit test execution
endif()



configure_file("setup.py.in" "setup.py")

add_subdirectory(morphotool)
add_subdirectory(test)

set(PY_INSTALL_OPTIONS "--prefix=${CMAKE_INSTALL_PREFIX}")

install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py install ${PY_INSTALL_OPTIONS} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")

endif()


