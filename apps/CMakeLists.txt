add_executable (convert convert.cpp)
target_include_directories(convert PUBLIC ../include ${morphio_shared_include_dirs})
target_link_libraries(convert morphio_shared)
