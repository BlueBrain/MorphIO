if(MORPHO_TOOL_DOCUMENTATION)
find_package(Pandoc)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/index.html
	COMMAND
	${PANDOC_EXECUTABLE} -s -S
	--self-contained
	-c ${CMAKE_CURRENT_SOURCE_DIR}/github-pandoc.css
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
	-o ${CMAKE_CURRENT_BINARY_DIR}/index.html
)

add_custom_target(doc ALL DEPENDS
	${CMAKE_CURRENT_BINARY_DIR}/index.html
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/index.html
    DESTINATION ${HTML_INSTALL_DIR}
)
endif(MORPHO_TOOL_DOCUMENTATION)
