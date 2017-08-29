from __future__ import absolute_import

import json
import os
import unittest

import numpy as np
from numpy import testing as nptest

from morphotool import MorphologyDB
from morphotool.morphotool import *

_CUR_DIR = os.path.dirname(__file__)
_H5V1_MORPHO_TEST_FILE = os.path.join(_CUR_DIR, "dend-C090905B_axon-sm110125a1-3_idA.h5")


class TestMorphoTree(unittest.TestCase):

    def test_serialization(self):
        fmt = SerializationFormat.JSON
        db = MorphologyDB(_CUR_DIR)
        tree = db["C010306C"]
        assert len(tree) > 0
        data_bytes = tree.serialize(fmt)

        # JSON is valid
        data_str = data_bytes if isinstance(data_bytes, str) \
            else data_bytes.decode("utf-8")
        data = json.loads(data_str)

        reloaded_tree = MorphoTree.from_bytes(data_bytes, fmt)
        assert len(tree) == len(reloaded_tree)

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
        assert not first_node.is_of_type(MORPHO_NODE_TYPE.neuron_section_type)

        soma = first_node
        assert len(soma.line_loop) == 1

        sphere_soma = soma.sphere
        ref_sphere = Types.Sphere((0,0,0), 0.5)
        assert ref_sphere.radius == sphere_soma.radius
        assert ref_sphere.center.close_to(sphere_soma.center)
        assert soma.section_type == NEURON_STRUCT_TYPE.soma
        assert len(tree.get_children(0)) == 0

        node2 = NeuronSection(NEURON_STRUCT_TYPE.dentrite_basal,
                             np.array([(0.1, 0.1, 0.1), (0.2, 0.2, 0.2)]),
                             [0.5, 0.5]
                )
        id = tree.add_node(0, node2)

        second_node = tree.get_node(1)
        assert id == 1
        assert tree.tree_size == 2
        assert second_node.is_of_type(MORPHO_NODE_TYPE.neuron_section_type)
        assert second_node.is_of_type(MORPHO_NODE_TYPE.neuron_node_3d_type)
        assert not second_node.is_of_type(MORPHO_NODE_TYPE.neuron_soma_type)

        section1 = second_node
        assert section1.section_type == NEURON_STRUCT_TYPE.dentrite_basal
        assert len(tree.get_children(0)) == 1
        assert tree.get_children(0)[0] == id
        assert tree.get_parent(1) == 0
        assert tree.get_parent(0) == -1

        # test swap method
        tree_copy = MorphoTree()
        assert tree_copy.tree_size == 0

        tree_copy.swap(tree)
        assert tree_copy.tree_size == 2
        assert tree.tree_size == 0


class Test_H5V1(unittest.TestCase):
    def setUp(self):
        reader = MorphoReader(_H5V1_MORPHO_TEST_FILE)
        self.tree = reader.create_morpho_tree()
        # force destroy the loader to check any data dependencies

    def test_reader(self):
        tree = self.tree
        reader = MorphoReader(_H5V1_MORPHO_TEST_FILE)

        reader_structs = reader.struct_raw
        self.assertEqual(len(reader_structs), tree.tree_size)
        self.assertTrue(tree.get_node(0).is_of_type(MORPHO_NODE_TYPE.neuron_soma_type))

        raw_points = reader.points_raw
        soma = tree.get_node(0)
        self.assertTrue(soma.section_type == NEURON_STRUCT_TYPE.soma)
        self.assertEqual(len(soma.line_loop), reader.get_section_range_raw(0)[1])

        # We keep C++ style to avoid introducing mismatches to the original version
        for i in range(1, tree.tree_size):
            nsection = tree.get_node(i)
            self.assertTrue(nsection.is_of_type(MORPHO_NODE_TYPE.neuron_section_type))
            section_raw_offset = reader.get_section_range_raw(i)[0]
            section_raw_size = reader.get_section_range_raw(i)[1]
            self.assertEqual(nsection.number_points, section_raw_size)
            self.assertEqual(len(nsection.points), section_raw_size)
            self.assertEqual(len(nsection.radius), section_raw_size)
            self.assertTrue(nsection.section_type in (NEURON_STRUCT_TYPE.axon,
                                                    NEURON_STRUCT_TYPE.dentrite_basal,
                                                    NEURON_STRUCT_TYPE.dentrite_apical))

            for j in range(nsection.number_points):
                # Raw points map to Numpy structures -> cool for indexing and math
                point_raw = (raw_points[section_raw_offset+j, 0:3])
                radius_raw = raw_points[section_raw_offset+j, 3] / 2
                nptest.assert_array_almost_equal(point_raw, nsection.points[j])
                nptest.assert_array_almost_equal(radius_raw, nsection.radius[j])


    @staticmethod
    def _is_inside_box(box, point):
        b_min = box.min_corner
        b_max = box.max_corner
        for i in range(3):
            if b_min[i] > point[i]:
                return False
            if b_max[i] < point[i]:
                return False
        return True


    def test_bounding_box(self):
        global_tree_box = self.tree.bounding_box
        for node in self.tree.all_nodes:
            for point in (node.pointsVector if isinstance(node, NeuronSection) else node.line_loop):
                self.assertTrue(global_tree_box, point)


    def test_spatial(self):
        index = SpatialIndex()
        index.add_morpho_tree(self.tree)
        for node in self.tree.all_nodes[1:]:
            for point in (node.pointsVector if isinstance(node, NeuronSection) else node.line_loop):
                self.assertTrue(index.is_within(point))


if __name__ == "__main__":
    unittest.main()
