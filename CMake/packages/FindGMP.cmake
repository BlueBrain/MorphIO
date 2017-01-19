#  find GMP Library
# -------------------------
#
#
#  GMP_DEFINITIONS		compiler flags for compiling with GMP
#  GMP_INCLUDE_DIRS             where to find gmp.h
#  GMP_LIBRARIES		the libraries needed to use GMP
#  GMP_FOUND			if false, do not try to use GMP
#
#


if (GMP_INCLUDE_DIRS AND GMP_LIBRARIES)
    # Already in cache, be silent

    set(GMP_FOUND TRUE)

else()

    find_path(GMP_INCLUDE_DIRS NAMES gmp.h )
    find_library(GMP_LIBRARIES NAMES gmp libgmp )


    message(STATUS "GMP libraries: ${GMP_LIBRARIES} " )


    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(GMP DEFAULT_MSG GMP_INCLUDE_DIRS GMP_LIBRARIES)

    mark_as_advanced(GMP_INCLUDE_DIRS GMP_LIBRARIES)


endif ()
