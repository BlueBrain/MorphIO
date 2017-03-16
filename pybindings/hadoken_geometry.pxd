# ======================================================================================================================
cdef extern from "hadoken/geometry/geometry.hpp" namespace "hadoken::geometry::cartesian":
# ----------------------------------------------------------------------------------------------------------------------
    cdef cppclass point_base[T, H]:
        pass

    cdef cppclass point3d:
        double * data()

    cdef cppclass vector3d:
        double * data()

    cdef cppclass linestring3d:
        double * data()

    cdef cppclass sphere3d:
        double * data()

    cdef cppclass cone3d:
        double * data()

    cdef cppclass circle3d:
        double * data()

    cdef cppclass box3d:
        double * data()