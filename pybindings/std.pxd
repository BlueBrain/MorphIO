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

cdef extern from "<utility>" namespace "std":
    cdef T&& move[T](T)
    cdef T&& forward[T](T)