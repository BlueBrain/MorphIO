# ======================================================================================================================
cdef extern from "morpho/morpho_tree.hpp" namespace "morpho::neuron_struct_type":
# ----------------------------------------------------------------------------------------------------------------------
    cdef enum neuron_struct_type "morpho::neuron_struct_type":
        soma
        axon
        dentrite_basal
        dentrite_apical
        unknown

