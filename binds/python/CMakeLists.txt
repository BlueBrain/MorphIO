pybind11_add_module(python_minimorph minimorph.cpp)

target_link_libraries(python_minimorph
    PRIVATE
    morphio
    HighFive
    GSL
    lexertl14
    )
