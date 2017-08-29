##############################################
# Requirements
#
find_package(PythonInterpLibs QUIET)
find_package(Cython 0.25.0)
find_package(NumPy QUIET)

if(NOT BUILD_PYTHON_MODULE)
  return()
endif()



##############################################
# Definitions
#
option(BUILD_PYTHON_DISTRIBUTABLE "Builds a Self-contained Python compiled module (wheel)" OFF)
set(cybindings_cpp "${CMAKE_CURRENT_SOURCE_DIR}/morphotool/morphotool.cpp")


# Getting the INCLUDE_DIRECTORIES to pass on to the compilation of the extension
get_target_property(_dirs morpho INCLUDE_DIRECTORIES)
set(lib_include_dirs .)
foreach(_dir ${_dirs})
    set(lib_include_dirs "${lib_include_dirs}:${_dir}")
endforeach()
set(lib_include_dirs "${lib_include_dirs}:${PYTHON_NUMPY_INCLUDE_DIR}")



##############################################
# Rules

#
# Main target -> Requires libmorpho and Cython bindings
# --------------------------------------
# We use a flag file since the main output filename is unknown
set(_pymod_flag ${CMAKE_CURRENT_BINARY_DIR}/.python_mod.flag)
add_custom_target(
    pymorphotool ALL 
    DEPENDS ${_pymod_flag}
)
add_custom_command(
    OUTPUT ${_pymod_flag}
    COMMAND
        # Uses setup.py to compile extension and link
        PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}
        ${PYTHON_EXECUTABLE} setup.py 
        build
          --build-base=${CMAKE_CURRENT_BINARY_DIR}/debug.build
        build_ext
          "-I${lib_include_dirs}"
          -L${CMAKE_BINARY_DIR}/src/morpho:${CMAKE_BINARY_DIR}/src/io
          -R${CMAKE_BINARY_DIR}/src/morpho:${CMAKE_BINARY_DIR}/src/io
        install 
          --install-lib=${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
        touch ${_pymod_flag}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${cybindings_cpp}
        morpho
    COMMENT 
        "Building Python module..."
)

#
# Python Source distributable package
# --------------------------------------
set(_sdist_flag ${CMAKE_CURRENT_BINARY_DIR}/.python_sdist.flag)
add_custom_target(
    python_sdist ALL 
    DEPENDS ${_sdist_flag}
)
add_custom_command(
    OUTPUT ${_sdist_flag}
    COMMAND ${PYTHON_EXECUTABLE} setup.py sdist --dist-dir ${CMAKE_BINARY_DIR}/dist > /dev/null
    COMMAND touch ${_sdist_flag}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS pymorphotool
    COMMENT "Building Python source distribution package..."
)

#
# Self-contained distributable package (single lib morphotool)
# --------------------------------------
if(BUILD_PYTHON_DISTRIBUTABLE)
# We dont use command since we dont know the output filename
set(_bdist_flag ${CMAKE_CURRENT_BINARY_DIR}/.python_bdist_wheel.flag)
add_custom_target(
    python_bdist_wheel ALL 
    DEPENDS ${_bdist_flag}
)
add_custom_command(
    OUTPUT ${_bdist_flag}
    COMMAND
        ${PYTHON_EXECUTABLE} setup.py
         build --build-base=${CMAKE_CURRENT_BINARY_DIR}/bdist_wheel.build
         build_ext -I${lib_include_dirs} -L${CMAKE_BINARY_DIR}/src -lhdf5
         bdist_wheel --dist-dir ${CMAKE_BINARY_DIR}/dist
         > /dev/null
    COMMAND
        touch ${_bdist_flag}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${cybindings_cpp} morphostatic
    COMMENT "Building Python Wheel..."
)
endif()


# Add rules for rebuilding bindings py extension
add_subdirectory(bindings)

# Hack around cmake bug 14633 - use a target to communicate file dependencies across cmake files
add_custom_command(OUTPUT ${cybindings_cpp} DEPENDS cybindings)



##############################################
# Install


# Install Python package linking to the lib at install destination
install(CODE "
     execute_process(COMMAND python setup.py
        build
          --build-base=${CMAKE_CURRENT_BINARY_DIR}/install.build
        build_ext
          -I${lib_include_dirs}
          -L${LIB_INSTALL_DIR}
          -R${LIB_INSTALL_DIR}
        install
          --prefix=${PYTHON_INSTALL_PREFIX}
          --old-and-unmanageable # Install wherever
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
     


# Install distributables
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/dist/
    DESTINATION python
)


# Messsage to help users installing python package in other locations
install(CODE "MESSAGE(STATUS \"${Cyan}All done. To install Python bindings \
to your current Python environment run 'pip install .' from the \
'python' directory${ColorReset}\")")




################################################
# TESTS
add_subdirectory(test)



