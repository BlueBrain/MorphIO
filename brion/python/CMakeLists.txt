include_directories(brion ../.. ../../Servus ../../3rdparty/vmmlib ..)

pybind11_add_module(python_brion brion.cpp ../morphology.cpp)
target_link_libraries(python_brion)
