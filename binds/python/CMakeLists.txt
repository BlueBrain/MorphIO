pybind11_add_module(morphio
    SYSTEM
    morphio.cpp
    bind_immutable.cpp
    bindings_utils.cpp
    bind_misc.cpp
    bind_mutable.cpp
    bind_vasculature.cpp
    )

target_include_directories(morphio
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
  )

target_link_libraries(morphio
    PRIVATE
    morphio_static)
