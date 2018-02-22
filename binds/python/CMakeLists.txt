pybind11_add_module(morphio morphio.cpp)

target_include_directories(morphio
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
  )

target_link_libraries(morphio
    PRIVATE
    morphio_static)
