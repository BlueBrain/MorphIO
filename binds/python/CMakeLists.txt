pybind11_add_module(python_morphio morphio.cpp)

target_link_libraries(python_morphio
    PRIVATE
    morphio
    )
