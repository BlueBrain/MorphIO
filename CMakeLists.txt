project(morpho-tool)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(MORPHOTOOL_VERSION_MAJOR "0")
set(MORPHOTOOL_VERSION_MINOR "5")
set(MORPHOTOOL_VERSION "${MORPHOTOOL_VERSION_MAJOR}.${MORPHOTOOL_VERSION_MINOR}")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(Display)


option(MORPHOTOOL_UNIT_TESTS "Enable or disable unit tests" ON)
option(ENABLE_MESHER_CGAL "Enable or disable integrated mesher based on CGAL" OFF)
option(BUILD_PYTHON_MODULE "Build a Python library" OFF)

set(Boost_NO_BOOST_CMAKE TRUE)

## find dependencies
find_package(Threads)
find_package(Boost 1.55 QUIET REQUIRED COMPONENTS system thread filesystem program_options)
find_package(HDF5 QUIET REQUIRED)
find_package(CLANG_TIDY)
# Eventually required for rebuilding bindings
find_package(Cython 0.25.0)


set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}" )

if(ENABLE_MESHER_CGAL)
    add_definitions( -DENABLE_MESHER_CGAL=1)
    set(MOPHO_MESHER_TARGET "morpho_mesher")
endif()



file(GLOB MORPHO_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/morpho/*)
file(GLOB MORPHO_HEADERS_BITS ${CMAKE_CURRENT_SOURCE_DIR}/include/morpho/*/*)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/highfive/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/hadoken/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/mvd-tool/include)


include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS})
add_definitions( -DH5_USE_BOOST -DMORPHO_VERSION_MAJOR=\"${MORPHOTOOL_VERSION_MAJOR}\" -DMORPHO_VERSION_MINOR=\"${MORPHOTOOL_VERSION_MINOR}\" )


install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/morpho DESTINATION ${INCLUDE_INSTALL_DIR})

option(CLANG_FORMAT_PRECOMMIT
	   "Ensure git staging area is formatted according to clang-format"
	   FALSE)
if(CLANG_FORMAT_PRECOMMIT)
    add_subdirectory(deps/git-cmake-format)
endif()

# Targets
add_subdirectory(src)
add_subdirectory(deps/hadoken)
add_subdirectory(deps/highfive)
add_subdirectory(tests/unit)
add_subdirectory(python)

