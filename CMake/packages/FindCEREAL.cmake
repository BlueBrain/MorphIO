#  find CEREAL Library
# ---------------------
#
#
#  CEREAL_DEFINITIONS		compiler flags for compiling with CEREAL
#  CEREAL_INCLUDE_DIR             where to find cereal/cereal.hpp
#  CEREAL_FOUND			if false, do not try to use CEREAL
#
#

if (CEREAL_INCLUDE_DIR)
    # Already in cache, be silent
    set(CEREAL_FOUND TRUE)
else()
    find_path(CEREAL_INCLUDE_DIR NAMES cereal/cereal.hpp)
    message(STATUS "cereal library: ${CEREAL_INCLUDE_DIR} " )

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(CEREAL DEFAULT_MSG CEREAL_INCLUDE_DIR)
    mark_as_advanced(CEREAL_INCLUDE_DIR)
endif ()
