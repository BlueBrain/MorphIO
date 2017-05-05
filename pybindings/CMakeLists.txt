configure_file("setup.py.in" "setup.py")

add_subdirectory("morphotool")
add_subdirectory("test")


install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py install WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")

