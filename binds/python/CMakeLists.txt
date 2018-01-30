include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/3rdparty/vmmlib)

pybind11_add_module(python_minimorph minimorph.cpp)
target_link_libraries(python_minimorph)
