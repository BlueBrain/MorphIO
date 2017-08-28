# ======================================================================================================================
cdef extern from "morpho/morpho_serialization.hpp" namespace "morpho":
    cdef cppclass serialization_format "morpho::serialization_format":
        pass

cdef extern from "morpho/morpho_serialization.hpp" namespace "morpho::serialization_format":
    cdef serialization_format BINARY, PORTABLE_BINARY, JSON, XML
