include(CMakeFindDependencyMacro)

find_dependency(gsl-lite)
find_dependency(HighFive)

include("${CMAKE_CURRENT_LIST_DIR}/MorphIOTargets.cmake")
