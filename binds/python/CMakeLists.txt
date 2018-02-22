pybind11_add_module(morphio morphio.cpp)

target_link_libraries(morphio
    PRIVATE
    morphiocpp)
