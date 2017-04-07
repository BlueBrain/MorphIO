# ======================================================================================================================
cdef extern from "hadoken/geometry/geometry.hpp" namespace "hadoken::geometry::cartesian":
# ----------------------------------------------------------------------------------------------------------------------

    cdef cppclass point3d:
        double* data()

    cdef cppclass vector3d:
        double* data()
        size_t size()

    cdef cppclass box3d:
        box3d(box3d&)
        point3d& min_corner()
        point3d& max_corner()

    cdef cppclass cone3d:
        cone3d(cone3d&)
        point3d get_center()
        double get_radius()

    cdef cppclass linestring3d:
        linestring3d(linestring3d&)
        double* data()
        size_t size()

    cdef cppclass sphere3d:
        sphere3d(sphere3d&)
        point3d get_center()
        double get_radius()

    cdef cppclass circle3d:
        circle3d(circle3d&)
        point3d get_center()
        double get_radius()

