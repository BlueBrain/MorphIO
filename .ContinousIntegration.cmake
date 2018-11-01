# Extra flag to be used for continuous integration
# This is taken into account when build the project with the following line:
# cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../.ContinousIntegration.cmake ..
set(CMAKE_CXX_FLAGS_INIT " -Werror -Wall -Wextra")
