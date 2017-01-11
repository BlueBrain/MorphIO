#  find CGAL Library
# -------------------------
#
#  Adapted from 
#  http://code.google.com/p/pixelstruct/source/browse/trunk/cmake/FindCGAL.cmake?r=29
#
#  CGAL_DEFINITIONS		compiler flags for compiling with CGAL
#  CGAL_INCLUDE_DIRS	where to find CGAL.h
#  CGAL_LIBRARIES		the libraries needed to use CGAL
#  CGAL_FOUND			if false, do not try to use CGAL
#
#


include(FindPackageHandleStandardArgs)



IF(CGAL_INCLUDE_DIRS AND CGAL_LIBRARIES)
    SET(CGAL_FOUND TRUE)
ELSE()
    FIND_PATH(CGAL_INCLUDE_DIRS CGAL/basic.h

    )
    FIND_LIBRARY(CGAL_LIBRARIES NAMES CGAL libCGAL

   )
    
    IF(CGAL_INCLUDE_DIRS AND CGAL_LIBRARIES)
        SET(CGAL_FOUND TRUE)
        MESSAGE(STATUS "Found CGAL Headers:  ${CGAL_INCLUDE_DIRS}")
		MESSAGE(STATUS "Found CGFAL Libraries: ${CGAL_LIBRARIES}")
    ELSE()
        SET(CGAL_FOUND FALSE)
        MESSAGE(STATUS "CGAL not found.")
    ENDIF()
    
    MARK_AS_ADVANCED(CGAL_INCLUDE_DIR CGAL_LIBRARIES)
ENDIF()


