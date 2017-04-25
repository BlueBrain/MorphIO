from libcpp cimport bool
from libcpp.vector cimport vector
# ======================================================================================================================
cdef extern from "hadoken/geometry/geometry.hpp" namespace "hadoken::geometry::cartesian":
# ----------------------------------------------------------------------------------------------------------------------

    cdef cppclass point3d:
        point3d()
        point3d(point3d)
        point3d(double, double, double)
        double* data()
        bool close_to(point3d&)

    cdef cppclass vector3d:
        double* data()
        size_t size()

    cdef cppclass box3d:
        box3d(box3d&)
        point3d& min_corner()
        point3d& max_corner()

    cdef cppclass cone3d:
        cone3d(cone3d&)
        point3d get_center[N]()
        double get_radius[N]()

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

