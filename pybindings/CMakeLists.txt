configure_file("setup.py.in" "setup.py")

add_subdirectory("morphotool")
add_subdirectory("test")

set(PY_INSTALL_OPTIONS "--prefix=${CMAKE_INSTALL_PREFIX}" CACHE STRING "setup.py install options")

install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py install ${PY_INSTALL_OPTIONS} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")

