add_library(HighFive INTERFACE)
target_include_directories(HighFive INTERFACE HighFive/include/highfive)

add_library(vmmlib INTERFACE)
target_include_directories(vmmlib INTERFACE vmmlib)

add_library(lunchbox INTERFACE)
target_include_directories(lunchbox INTERFACE lunchbox)
