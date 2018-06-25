import os
import numpy as np
from itertools import combinations
from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio import Morphology, upstream

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


# These 3 cells are identical
CELLS = {
    'asc': Morphology(os.path.join(_path, "simple.asc")),
    'swc': Morphology(os.path.join(_path, "simple.swc")),
    'h5': Morphology(os.path.join(_path, "h5/v1/simple.h5")),
}


def test_equality():
    for cell1, cell2 in combinations(['asc', 'swc', 'h5'], 2):
        ok_(CELLS[cell1] == CELLS[cell2], '{} != {}'.format(cell1, cell2))


def test_is_root():
    for _, cell in CELLS.items():
        ok_(all(section.is_root for section in cell.root_sections))
        ok_(all(not child.is_root
                for section in cell.root_sections
                for child in section.children))


def test_iter():
    for cell in CELLS.values():
        assert_array_equal([section.points for section in
                            cell.root_sections[0].children[0].iter(upstream)],
                           [[[0.,  5.,  0.],
                             [-5.,  5.,  0.]],
                            [[0., 0., 0.],
                             [0., 5., 0.]]])


def test_mitochondria():
    morpho = Morphology(os.path.join(_path, "h5/v1/mitochondria.h5"))
    mito = morpho.mitochondria
    assert_equal(len(mito.root_sections), 2)
    mito_root = mito.root_sections

    assert_array_equal(mito_root[0].diameters,
                       [10, 20])
    assert_array_equal(mito_root[0].relative_path_lengths,
                       np.array([0.5, 0.6], dtype=np.float32))
    assert_array_equal(mito_root[0].neurite_section_ids,
                       np.array([0., 0.], dtype=np.float32))

    assert_equal(len(mito_root[0].children), 1)
    mito_child = mito_root[0].children[0]
    assert_equal(mito_child.parent.id,
                 mito_root[0].id)

    assert_array_equal(mito_child.diameters,
                       [20, 30, 40, 50])
    assert_array_equal(mito_child.relative_path_lengths,
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))
    assert_array_equal(mito_child.neurite_section_ids,
                       np.array([3, 4, 4, 5], dtype=np.float32))

    assert_array_equal(mito_root[1].diameters,
                       [5, 6, 7, 8])
    assert_array_equal(mito_root[1].relative_path_lengths,
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))
    assert_array_equal(mito_root[1].neurite_section_ids,
                       np.array([0, 1, 1, 2], dtype=np.float32))

    assert_equal(len(mito_root[1].children), 0)
