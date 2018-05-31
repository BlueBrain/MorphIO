find_package(HDF5 REQUIRED)
add_library(HighFive INTERFACE)
target_include_directories(HighFive INTERFACE
    HighFive/include/
    ${HDF5_INCLUDE_DIRS})
target_link_libraries(HighFive INTERFACE
    ${HDF5_C_LIBRARIES})

add_subdirectory(pybind11)
target_include_directories(pybind11 INTERFACE
    pybind11)

add_subdirectory(GSL)
target_include_directories(GSL INTERFACE
    GSL)

add_subdirectory(eigen)

add_library(lexertl14 INTERFACE)
target_include_directories(lexertl14 INTERFACE
    lexertl14)
