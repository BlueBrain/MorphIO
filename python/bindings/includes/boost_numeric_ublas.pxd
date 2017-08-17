cimport std
# ======================================================================================================================
cdef extern from "boost/numeric/ublas/matrix.hpp" namespace "boost::numeric::ublas":
# ----------------------------------------------------------------------------------------------------------------------
    cdef cppclass unbounded_array[T]:
        T* begin()
        T* end()

    cdef cppclass matrix[T]:
        matrix(matrix&)
        std.size_t size1()
        std.size_t size2()
        unbounded_array[T]& data()

    cdef cppclass matrix_range[T]:
        pass


# ======================================================================================================================
cdef extern from "boost/numeric/ublas/vector.hpp" namespace "boost::numeric::ublas":
# ----------------------------------------------------------------------------------------------------------------------
    cdef cppclass vector[T]:
        pass

    cdef cppclass vector_range[T]:
        pass
