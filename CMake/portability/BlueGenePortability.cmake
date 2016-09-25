##
## Portability check on Blue Gene Q environment

if(IS_DIRECTORY "/bgsys")
    set(BLUEGENE TRUE)
endif()


if(BLUEGENE)
	# define library type to static on BGQ
	set(COMPILE_LIBRARY_TYPE "STATIC")
	
	## Blue Gene/Q do not support linking with MPI library when compiled with mpicc wrapper
        ## we disable any MPI_X_LIBRARY linking and rely on mpicc wrapper
	set(MPI_LIBRARIES "")
	set(MPI_C_LIBRARIES "")	
	set(MPI_CXX_LIBRARIES "")

	## static linking need to be forced on BlueGene
	# Boost need a bit of tuning parameters for static linking
	set(Boost_NO_BOOST_CMAKE TRUE)
        set(Boost_USE_STATIC_LIBS TRUE)	
else()

if(NOT DEFINED COMPILE_LIBRARY_TYPE)
    set(COMPILE_LIBRARY_TYPE "SHARED")
endif()

endif()


# cmake > 3.0 add -Wl,-BDynamic for any linking argument
# which does not match the library pattern *.a
# this is annoying and cause the pkg_search_module _LIBRARIES to bug
# in case of static linking
# this macro fix this issue once for all 
macro(fix_bgq_static_linking INPUT_LIST OUTPUT_LIST )
        if("${CMAKE_VERSION}" VERSION_GREATER "3.0")

		set(LOCAL_LIST)

		LIST(APPEND LOCAL_LIST "-Wl,-Bstatic")
		LIST(APPEND LOCAL_LIST "${INPUT_LIST}")

		SET(${OUTPUT_LIST} ${LOCAL_LIST})
	else()
		set(${OUTPUT_LIST} "${INPUT_LIST}")

	endif()

endmacro()



