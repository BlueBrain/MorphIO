# ======================================================================================================================
cdef extern from "hadoken/geometry/geometry.hpp" namespace "hadoken::geometry::cartesian":
# ----------------------------------------------------------------------------------------------------------------------

    cdef cppclass point3d:
        double * data()

    cdef cppclass vector3d:
        double * data()

    cdef cppclass box3d:
        point3d& min_corner()
        point3d& max_corner()

    cdef cppclass cone3d:
        point3d get_center()
        double get_radius()

    cdef cppclass linestring3d:
        double * data()

    cdef cppclass sphere3d:
        double * data()

    cdef cppclass circle3d:
        double * data()

