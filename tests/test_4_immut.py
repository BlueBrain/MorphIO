import os
import numpy as np
from collections import OrderedDict
from itertools import combinations
from numpy.testing import assert_array_equal, assert_array_almost_equal
from nose.tools import assert_equal, assert_not_equal, assert_raises, ok_

from morphio import Morphology, upstream, IterType, RawDataError
import morphio.vasculature as vasculature

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


# These 3 cells are identical
CELLS = OrderedDict({
    'asc': Morphology(os.path.join(_path, "simple.asc")),
    'swc': Morphology(os.path.join(_path, "simple.swc")),
    'h5': Morphology(os.path.join(_path, "h5/v1/simple.h5")),
})


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
    neuron = Morphology(os.path.join(_path, "iterators.asc"))
    root = neuron.root_sections[0]
    assert_array_equal([section.id for section in root.iter(IterType.depth_first)],
                       [0,1,2,3,4,5,6])
    assert_array_equal([section.id for section in root.iter(IterType.breadth_first)],
                       [0, 1, 4, 2, 3, 5, 6])

    assert_array_equal([section.id for section in neuron.iter(IterType.breadth_first)],
                       [0, 1, 4, 2, 3, 5, 6, 7, 8, 9])


    for _, cell in CELLS.items():
        assert_array_equal([section.id for section in cell.iter()],
                           [0, 1, 2, 3, 4, 5])
        assert_array_equal([section.id for section in cell.iter(IterType.depth_first)],
                           [0, 1, 2, 3, 4, 5])
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
    assert_equal(mito.root_sections[0].id, 0)
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


def test_empty_vasculature():
    assert_raises(RawDataError, vasculature.Vasculature, os.path.join(_path, "h5/empty_vasculature.h5"))


def test_equality_vasculature():
    morphology1 = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
    morphology1_bis = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
    morphology2 = vasculature.Vasculature(os.path.join(_path, "h5/vasculature2.h5"))

    assert_equal(morphology1, morphology1_bis)
    assert_not_equal(morphology1, morphology2)


def test_components_vasculature():
    morphology = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
    assert_array_almost_equal(morphology.section(0).points,
                       np.array([[1265.47399902,  335.42364502, 1869.19274902],
                              [1266.26647949,  335.57000732, 1869.74914551],
                              [1267.09082031,  335.68869019, 1870.31469727],
                              [1267.89404297,  335.78134155, 1870.91418457],
                              [1268.67077637,  335.85733032, 1871.54992676],
                              [1269.42773438,  335.92602539, 1872.21008301],
                              [1270.17431641,  335.99368286, 1872.88195801],
                              [1270.92016602,  336.06558228, 1873.55395508],
                              [1271.6739502 ,  336.14227295, 1874.21740723],
                              [1272.44091797,  336.23706055, 1874.86047363],
                              [1273.22216797,  336.31613159, 1875.49523926],
                              [1274.        ,  336.70001221, 1876.        ]]))

    assert_array_almost_equal(morphology.section(0).diameters,
                              np.array([1.96932483, 1.96932483, 1.96932483, 1.96932483, 1.96932483,
                                        1.96932483, 1.96932483, 1.96932483, 1.96932483, 1.96932483,
                                        1.96932483, 2.15068388]))

    assert_equal(len(morphology.sections), 3080)
    assert_equal(len(morphology.points), 55807)
    assert_equal(len(morphology.diameters), 55807)
    assert_array_almost_equal(morphology.diameters[-5:],
                              np.array([0.78039801, 0.78039801, 0.78039801, 2.11725187, 2.11725187]))
    assert_equal(len(morphology.section_types), 3080)
    assert_equal(len(morphology.section(0).predecessors), 0)
    assert_equal(len(morphology.section(0).successors), 2)
    assert_equal(morphology.section(0).successors[0].id, 1)
    assert_equal(morphology.section(0).successors[1].id, 2)


def test_iterators_vasculature():
    morphology = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
    assert_array_equal([sec.id for sec in morphology.sections], range(3080))
    assert_equal(len([section.id for section in morphology.iter()]), 3080)
    all_sections = set([sec.id for sec in morphology.sections])
    for sec in morphology.iter():
        all_sections.remove(sec.id)
    assert_equal(len(all_sections), 0)

def test_section___str__():
    assert_equal(str(CELLS['asc'].root_sections[0]),
                 'Section(id=0, points=[(0 0 0),..., (0 5 0)])')
