find_package(HDF5 REQUIRED)

if (NOT EXTERNAL_HIGHFIVE)
  set(USE_BOOST OFF)
  set(HIGHFIVE_USE_BOOST OFF)
  add_subdirectory(HighFive EXCLUDE_FROM_ALL)
  target_include_directories(HighFive SYSTEM INTERFACE)
endif()

add_subdirectory(GSL_LITE)
target_include_directories(gsl-lite SYSTEM INTERFACE)

add_library(lexertl INTERFACE)
target_include_directories(lexertl SYSTEM INTERFACE lexertl)

if(BUILD_BINDINGS)
  add_subdirectory(pybind11)
  target_include_directories(pybind11 SYSTEM INTERFACE
    pybind11)
endif()
