find_package(HDF5 REQUIRED)
add_library(HighFive INTERFACE)
target_include_directories(HighFive SYSTEM INTERFACE
    HighFive/include/
    ${HDF5_INCLUDE_DIRS})
target_link_libraries(HighFive INTERFACE
    ${HDF5_C_LIBRARIES})

add_subdirectory(GSL_LITE)
target_include_directories(gsl-lite SYSTEM INTERFACE)

add_library(lexertl INTERFACE)
target_include_directories(lexertl SYSTEM INTERFACE lexertl)

if(BUILD_BINDINGS)
  add_subdirectory(pybind11)
  target_include_directories(pybind11 SYSTEM INTERFACE
    pybind11)
endif()
