# Extra flag to be used for continuous integration
# This is taken into account when build the project with the following line:
# cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../.ContinousIntegration.cmake ..
set(CMAKE_CXX_FLAGS_INIT "-Wall -Wcast-align -Wconversion -Wdouble-promotion -Werror -Wextra -Wformat=2 -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wshadow -Wsign-conversion -Wunused -Wuseless-cast")
