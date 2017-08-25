project(morpho-tool)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(MORPHOTOOL_VERSION_MAJOR "0")
set(MORPHOTOOL_VERSION_MINOR "5")
set(MORPHOTOOL_VERSION "${MORPHOTOOL_VERSION_MAJOR}.${MORPHOTOOL_VERSION_MINOR}")


## CMAKE config
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(Display)


## Optional components
option(MORPHOTOOL_UNIT_TESTS "Enable or disable unit tests" ON)
option(BUILD_PYTHON_MODULE "Build the Python module" ON)


## Dependencies
set(Boost_NO_BOOST_CMAKE TRUE)
find_package(Threads QUIET)
find_package(Boost 1.55 QUIET REQUIRED COMPONENTS system thread filesystem program_options)
find_package(HDF5 QUIET REQUIRED)
find_package(CLANG_TIDY)
find_package(CEREAL QUIET REQUIRED)


# Globals
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}" )
add_definitions( -DH5_USE_BOOST -DMORPHO_VERSION_MAJOR=\"${MORPHOTOOL_VERSION_MAJOR}\" -DMORPHO_VERSION_MINOR=\"${MORPHOTOOL_VERSION_MINOR}\" )

file(GLOB MORPHO_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/morpho/*)
file(GLOB MORPHO_HEADERS_BITS ${CMAKE_CURRENT_SOURCE_DIR}/include/morpho/*/*)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/highfive/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/hadoken/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/mvd-tool/include)
include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS} ${CEREAL_INCLUDE_DIR})


## INSTALL headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/morpho DESTINATION ${INCLUDE_INSTALL_DIR})

option(CLANG_FORMAT_PRECOMMIT "Ensure git staging area is formatted according to clang-format" FALSE)
if(CLANG_FORMAT_PRECOMMIT)
    add_subdirectory(deps/git-cmake-format)
endif()

## Components
add_subdirectory(src)
add_subdirectory(deps/hadoken EXCLUDE_FROM_ALL)
add_subdirectory(deps/highfive EXCLUDE_FROM_ALL)
add_subdirectory(tests/unit)
add_subdirectory(python)
add_subdirectory(doc)
