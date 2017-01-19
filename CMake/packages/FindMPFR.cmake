#  find MPFR Library
# -------------------------
#
#
#  MPFR_DEFINITIONS		compiler flags for compiling with MPFR
#  MPFR_INCLUDE_DIRS             where to find MPFR.h
#  MPFR_LIBRARIES		the libraries needed to use MPFR
#  MPFR_FOUND			if false, do not try to use MPFR
#
#


if (MPFR_INCLUDE_DIRS AND MPFR_LIBRARIES)
    # Already in cache, be silent

    set(MPFR_FOUND TRUE)

else()

    find_path(MPFR_INCLUDE_DIRS NAMES mpfr.h )
    find_library(MPFR_LIBRARIES NAMES mpfr libmpfr )


    message(STATUS "mpfr libraries: ${MPFR_LIBRARIES} " )


    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(MPFR DEFAULT_MSG MPFR_INCLUDE_DIRS MPFR_LIBRARIES)

    mark_as_advanced(MPFR_INCLUDE_DIRS MPFR_LIBRARIES)


endif ()
