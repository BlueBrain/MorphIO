from .includes cimport stats

# ======================================================================================================================
# Python bindings to namespace morpho::stats
# ======================================================================================================================

cdef class Stats:
    """This class is a namespace for statistic (static) functions"""

    @staticmethod
    def total_number_branches(MorphoTree tree):
        return stats.total_number_branches(deref(tree.ptr()))

    @staticmethod
    def total_number_point(MorphoTree tree):
        return stats.total_number_point(deref(tree.ptr()))

    @staticmethod
    def min_radius_segment(MorphoTree tree):
        return stats.min_radius_segment(deref(tree.ptr()))

    @staticmethod
    def max_radius_segment(MorphoTree tree):
        return stats.max_radius_segment(deref(tree.ptr()))

    @staticmethod
    def median_radius_segment(MorphoTree tree):
        return stats.median_radius_segment(deref(tree.ptr()))

    @staticmethod
    def has_duplicated_points(MorphoTree tree):
        return stats.has_duplicated_points(deref(tree.ptr()))
