# ======================================================================================================================
cdef extern from "morpho/morpho_tree.hpp" namespace "morpho::morpho_node_type":
# ----------------------------------------------------------------------------------------------------------------------
    cdef enum morpho_node_type "morpho::morpho_node_type":
        unknown
        neuron_node_3d_type
        neuron_section_type
        neuron_soma_type
