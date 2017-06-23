if(BUILD_PYTHON_MODULE)

option(BUILD_PYTHON_DISTRIBUTABLE "Builds a Self-contained Python compiled module (wheel)" OFF)

# Even though we dont reqire the lib, we need headers
find_package(PythonInterpLibs)

# Rebuild Cython?
option(REBUILD_BINDINGS "Rebuild Python bindings from Cython sources" OFF)
set(CYBINDINGS_CPP "${CMAKE_CURRENT_SOURCE_DIR}/morphotool/morphotool.cpp")

# Getting the INCLUDE_DIRECTORIES to pass on to the compilation of the extension
get_target_property(dirs morpho INCLUDE_DIRECTORIES)
set(dirstr .)
foreach(dir ${dirs})
    set(dirstr "${dirstr}:${dir}")
endforeach()


# Main target -> Requires libmorpho and Cython bindings
# Uses setup.py to compile extension and link
add_custom_target(pymorphotool ALL
    COMMAND ${PYTHON_EXECUTABLE} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/tmp1 build_ext "-I${dirstr}" -L${CMAKE_BINARY_DIR}/src/morpho:${CMAKE_BINARY_DIR}/src/io -R${CMAKE_BINARY_DIR}/src/morpho:${CMAKE_BINARY_DIR}/src/io -lmorpho_io install --install-lib=${CMAKE_CURRENT_BINARY_DIR} --old-and-unmanageable > /dev/null
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CYBINDINGS_CPP} morpho
    COMMENT "Writing morphotool Python module to ${CMAKE_CURRENT_BINARY_DIR}"
)
# Hack around cmake bug 14633 - use a target to communicate file dependencies across cmake files
add_custom_command(OUTPUT ${CYBINDINGS_CPP} DEPENDS cybindings_cpp)


# Self-contained distributable package (single lib morphotool)
if(BUILD_PYTHON_DISTRIBUTABLE)
set(libs_required_morpho hdf5)
add_custom_target(pydist ALL
    COMMAND ${PYTHON_EXECUTABLE} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/tmp2 build_ext "-I${dirstr}" -L${CMAKE_BINARY_DIR}/src "-l$<JOIN:${libs_required_morpho}, -l>" bdist_wheel --dist-dir ${CMAKE_BINARY_DIR} > /dev/null
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CYBINDINGS_CPP} morphostatic
    COMMENT "Building distributable Python wheel in ${CMAKE_BINARY_DIR}"
)
endif()


# Python Source distributable package
add_custom_target(pysdist
    COMMAND ${PYTHON_EXECUTABLE} setup.py sdist --dist-dir ${CMAKE_BINARY_DIR} > /dev/null
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS pymorphotool
    COMMENT "Building sdist package in ${CMAKE_BINARY_DIR}"
)


# If at build time CYBINDINGS_CPP doesnt exist, it wil be generated
add_subdirectory(bindings)

# TESTS
add_subdirectory(test)


# INSTALL
# Defaults to install in the CMAKE_INSTALL_PREFIX. Setuptools requires --old-and-unmanageable to create a plain package and not complain for the PYTHONPATH
set(PY_INSTALL_OPTIONS "--install-lib=${CMAKE_INSTALL_PREFIX} --old-and-unmanageable")
# Install Python package linking to the lib at install destination
install(CODE "execute_process(COMMAND python setup.py build_ext -I${dirstr} -L${CMAKE_INSTALL_PREFIX}/lib64 -R${CMAKE_INSTALL_PREFIX}/lib64 -lmorpho_io install ${PY_INSTALL_OPTIONS} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_QUIET)")
# Messsage to help users installing python package in other locations
install(CODE "MESSAGE(STATUS \"${Cyan}All done. To install Python bindings to your current Python environment run 'pip install .' from the 'python' directory${ColorReset}\")")
endif()
