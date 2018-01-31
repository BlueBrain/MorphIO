find_package(HDF5 REQUIRED)
add_library(HighFive INTERFACE)
target_include_directories(HighFive INTERFACE
    HighFive/include/
    ${HDF5_INCLUDE_DIRS})
target_link_libraries(HighFive INTERFACE
    ${HDF5_C_LIBRARIES})

add_library(vmmlib INTERFACE)
target_include_directories(vmmlib INTERFACE
    vmmlib)

add_subdirectory(pybind11)
target_include_directories(pybind11 INTERFACE
    pybind11)
