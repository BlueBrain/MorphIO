include(CMakeFindDependencyMacro)

find_dependency(gsl-lite)

include("${CMAKE_CURRENT_LIST_DIR}/MorphIOTargets.cmake")
