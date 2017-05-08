from __future__ import absolute_import
import unittest
from morphotool.morphotool import *
import numpy as np

class TestMorphoTree(unittest.TestCase):

    def test_mtree(self):

        tree = MorphoTree()
        assert tree.tree_size == 0

        node = NeuronSoma((0,0,0), 0.5)
        id = tree.add_node(-1, node)
        assert id == 0
        assert tree.tree_size == 1

        first_node = tree.get_node(0)
        assert first_node.is_of_type(MORPHO_NODE_TYPE.neuron_soma_type)
        assert first_node.is_of_type(MORPHO_NODE_TYPE.neuron_node_3d_type)
        assert not first_node.is_of_type(MORPHO_NODE_TYPE.neuron_branch_type)

        soma = first_node
        assert len(soma.line_loop) == 1

        sphere_soma = soma.sphere
        ref_sphere = Types.Sphere((0,0,0), 0.5)
        assert ref_sphere.radius == sphere_soma.radius
        assert ref_sphere.center.close_to(sphere_soma.center)
        assert soma.branch_type == NEURON_STRUCT_TYPE.soma
        assert len(tree.get_children(0)) == 0

        node2 = NeuronBranch(NEURON_STRUCT_TYPE.dentrite_basal,
                             np.array([(0.1, 0.1, 0.1), (0.2, 0.2, 0.2)]),
                             [0.5, 0.5]
                )
        id = tree.add_node(0, node2)

        second_node = tree.get_node(1)
        assert id == 1
        assert tree.tree_size == 2
        assert second_node.is_of_type(MORPHO_NODE_TYPE.neuron_branch_type)
        assert second_node.is_of_type(MORPHO_NODE_TYPE.neuron_node_3d_type)
        assert not second_node.is_of_type(MORPHO_NODE_TYPE.neuron_soma_type)

        branch1 = second_node
        assert branch1.branch_type == NEURON_STRUCT_TYPE.dentrite_basal
        assert len(tree.get_children(0)) == 1
        assert tree.get_children(0)[0] == id
        assert tree.get_parent(1) == 0
        assert tree.get_parent(0) == -1

        # test swapmethod
        tree_copy = MorphoTree()
        assert tree_copy.tree_size == 0

        tree_copy.swap(tree)
        assert tree_copy.tree_size == 2
        assert tree.tree_size == 0


if __name__ == "__main__":
    unittest.main()
