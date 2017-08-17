from libcpp.map cimport map
from libcpp.set cimport set
from libcpp.string cimport string
from libcpp.queue cimport queue
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp cimport pair
from libcpp.memory cimport unique_ptr, shared_ptr

cdef extern from "exception" namespace "std":
    cdef cppclass exception:
        pass

cdef extern from "stddef.h" namespace "std":
    ctypedef int size_t

#cdef extern from "<utility>" namespace "std":
#    T&& std_move "std::move"[T] (T &t)

cdef extern from "<memory>" namespace "std":
    shared_ptr[T] static_pointer_cast[T](...)

cdef extern from "<array>" namespace "std":
    cdef cppclass array[T, N]:
        T& operator[](int)
