cimport std
from libcpp cimport bool
from . cimport morpho
from . cimport stats

# ======================================================================================================================
cdef extern from "morpho/morpho_stats.hpp" namespace "morpho::stats":
# ----------------------------------------------------------------------------------------------------------------------
    std.size_t total_number_branches(const morpho.morpho_tree&)
    std.size_t total_number_point(const morpho.morpho_tree&)
    double min_radius_segment(const morpho.morpho_tree&)
    double max_radius_segment(const morpho.morpho_tree&)
    double median_radius_segment(const morpho.morpho_tree&)
    bool has_duplicated_points(const morpho.morpho_tree&)