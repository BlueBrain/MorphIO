project(morpho-tool)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(VERSION_MAJOR "1")
set(VERSION_MINOR "1")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)


option(MORPHOTOOL_UNIT_TESTS "Enable or disable unit tests" ON)
set(Boost_NO_BOOST_CMAKE TRUE)

## find dependencies
find_package(Boost 1.41 QUIET REQUIRED COMPONENTS system)
find_package(HDF5 QUIET REQUIRED)


add_definitions( -std=c++11 )


file(GLOB MORPHO_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/morpho/*)
file(GLOB MORPHO_HEADERS_BITS ${CMAKE_CURRENT_SOURCE_DIR}/include/morpho/*/*)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/highfive/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/hadoken/include)


include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS})
add_definitions( -DH5_USE_BOOST -DMORPHO_VERSION_MAJOR=\"${VERSION_MAJOR}\" -DMORPHO_VERSION_MINOR=\"${VERSION_MINOR}\" )

aux_source_directory(src SRC_LIST)
add_executable(morpho-tool ${SRC_LIST} ${MORPHO_HEADERS_BITS} ${MORPHO_HEADERS})
target_link_libraries(morpho-tool ${HDF5_C_LIBRARIES} ${HDF5_HL_LIBRARIES})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/morpho DESTINATION ${INCLUDE_INSTALL_DIR})
install(TARGETS morpho-tool RUNTIME DESTINATION ${BIN_INSTALL_DIR})

#add_subdirectory(tests/unit)
