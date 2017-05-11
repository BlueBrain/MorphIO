# Find the Python NumPy package
# PYTHON_NUMPY_INCLUDE_DIR
# PYTHON_NUMPY_FOUND
# will be set by this script


find_package( PythonInterp )

if(NOT PYTHON_EXECUTABLE)
    find_package(PythonInterp)
endif()


if (PYTHON_EXECUTABLE)
  # Find out the include path
  # also test return code
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c
            "from __future__ import print_function;import numpy; print(numpy.get_include(), end='');"
            OUTPUT_VARIABLE __numpy_path
			RESULT_VARIABLE PYTHON_NUMPY_RESULT_VAR)

  # And the version
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c
            "from __future__ import print_function\ntry: import numpy; print(numpy.__version__, end='')\nexcept:pass\n"
    OUTPUT_VARIABLE __numpy_version)

	if(PYTHON_NUMPY_RESULT_VAR STREQUAL "0")
		set(PYTHON_NUMPY_FOUND TRUE CACHED )
	endif()
elseif(__numpy_out)
  message(STATUS "Python executable not found.")
endif(PYTHON_EXECUTABLE)

find_path(PYTHON_NUMPY_INCLUDE_DIR numpy/arrayobject.h
  HINTS "${__numpy_path}" "${PYTHON_INCLUDE_PATH}" NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NumPy REQUIRED_VARS PYTHON_NUMPY_INCLUDE_DIR PYTHON_NUMPY_FOUND
                                        VERSION_VAR __numpy_version)
