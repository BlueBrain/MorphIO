add_library(HighFive INTERFACE)
target_include_directories(HighFive INTERFACE HighFive/include/)

add_library(vmmlib INTERFACE)
target_include_directories(vmmlib INTERFACE vmmlib)

add_subdirectory(pybind11)
target_include_directories(pybind11 INTERFACE pybind11)
