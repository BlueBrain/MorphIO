import os
from collections import OrderedDict

import numpy as np
import pytest
from numpy.testing import assert_array_almost_equal, assert_array_equal, assert_equal
from pathlib import Path

import morphio
from morphio import SectionType, IterType, Morphology, GlialCell, CellFamily, RawDataError, DendriticSpine

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


# These 3 cells are identical
CELLS = OrderedDict({
    'asc': Morphology(os.path.join(_path, "simple.asc")),
    'swc': Morphology(os.path.join(_path, "simple.swc")),
    'h5': Morphology(os.path.join(_path, "h5/v1/simple.h5")),
})


def test_is_root():
    for _, cell in CELLS.items():
        assert all(section.is_root for section in cell.root_sections)
        assert all(not child.is_root
                for section in cell.root_sections
                for child in section.children)


def test_distance():
    for _, cell in CELLS.items():
        assert cell.soma.max_distance == 0.


def test_iter():
    neuron = Morphology(os.path.join(_path, "iterators.asc"))
    root = neuron.root_sections[0]
    assert_array_equal([section.id for section in root.iter(IterType.depth_first)],
                       [0, 1, 2, 3, 4, 5, 6])
    assert_array_equal([section.id for section in root.iter(IterType.breadth_first)],
                       [0, 1, 4, 2, 3, 5, 6])

    assert_array_equal([section.id for section in neuron.iter(IterType.breadth_first)],
                       [0, 7, 1, 4, 8, 9, 2, 3, 5, 6])


    for _, cell in CELLS.items():
        assert_array_equal([section.id for section in cell.iter()],
                           [0, 1, 2, 3, 4, 5])
        assert_array_equal([section.id for section in cell.iter(IterType.depth_first)],
                           [0, 1, 2, 3, 4, 5])
        assert_array_equal([section.points for section in
                            cell.root_sections[0].children[0].iter(IterType.upstream)],
                           [[[0., 5., 0.],
                             [-5., 5., 0.]],
                            [[0., 0., 0.],
                             [0., 5., 0.]]])

def test_section_offsets():
    for cell in CELLS:
        assert_array_equal(CELLS[cell].section_offsets, [0, 2, 4, 6, 8, 10, 12])


def test_connectivity():
    for cell in CELLS:
        assert CELLS[cell].connectivity == {-1: [0, 3], 0: [1, 2], 3: [4, 5]}


def test_mitochondria():
    morpho = Morphology(os.path.join(_path, "h5/v1/mitochondria.h5"))
    mito = morpho.mitochondria
    assert len(mito.root_sections) == 2
    assert mito.root_sections[0].id == 0
    mito_root = mito.root_sections

    assert_array_equal(mito_root[0].diameters,
                       [10, 20])
    assert_array_equal(mito_root[0].relative_path_lengths,
                       np.array([0.5, 0.6], dtype=np.float32))
    assert_array_equal(mito_root[0].neurite_section_ids,
                       np.array([0., 0.], dtype=np.float32))

    assert len(mito_root[0].children) == 1
    mito_child = mito_root[0].children[0]
    assert (mito_child.parent.id ==
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

    assert len(mito_root[1].children) == 0


def test_endoplasmic_reticulum():
    morpho = Morphology(os.path.join(_path, "h5/v1/endoplasmic-reticulum.h5"))
    er = morpho.endoplasmic_reticulum
    assert_array_equal(er.section_indices, [1, 4, 5])
    assert_array_almost_equal(er.volumes, [10.55, 47.12, 0.83])
    assert_array_almost_equal(er.surface_areas, [111.24, 87.44, 0.11], decimal=5)
    assert_array_equal(er.filament_counts, [12, 42, 8])


def test_section___str__():
    assert (str(CELLS['asc'].root_sections[0]) ==
                 'Section(id=0, points=[(0 0 0),..., (0 5 0)])')


def test_from_pathlib():
    neuron = Morphology(Path(_path, "simple.asc"))
    assert len(neuron.root_sections) == 2


def test_more_iter():
    '''This used to fail at commit f74ce1f56de805ebeb27584051bbbb3a65cd1213'''
    m = Morphology(os.path.join(_path, 'simple.asc'))

    sections = list(m.iter())
    assert_array_equal([s1.id for s1 in sections],
                       [0, 1, 2, 3, 4, 5])

    sections = list(m.iter(IterType.breadth_first))
    assert_array_equal([s.id for s in sections],
                       [0, 3, 1, 2, 4, 5])

    sections = list(m.section(2).iter(IterType.upstream))
    assert_array_equal([s.id for s in sections],
                       [2, 0])


def test_glia():
    # check the glia section types
    assert_equal(int(SectionType.glia_perivascular_process), 2)
    assert_equal(int(SectionType.glia_process), 3)

    g = GlialCell(os.path.join(_path, 'astrocyte.h5'))
    assert g.cell_family == CellFamily.GLIA
    assert g.sections[100].type == SectionType.glia_perivascular_process
    assert g.sections[1000].type == SectionType.glia_process

    g = GlialCell(Path(_path, 'astrocyte.h5'))
    assert g.cell_family == CellFamily.GLIA

    with pytest.raises(RawDataError):
        GlialCell(Path(_path, 'simple.swc'))
    with pytest.raises(RawDataError):
        GlialCell(Path(_path, 'h5/v1/simple.h5'))


def test_dendritic_spine():
    assert_equal(int(SectionType.spine_neck), 2)
    assert_equal(int(SectionType.spine_head), 3)

    spine_path = os.path.join(_path, 'h5/v1/simple-dendritric-spine.h5')
    d = DendriticSpine(spine_path)
    assert d.cell_family == CellFamily.SPINE
    assert d.sections[0].type == SectionType.spine_head
    assert d.sections[1].type == SectionType.spine_neck

    assert len(d.post_synaptic_density) == 2
    assert d.post_synaptic_density[0].section_id == 1
    assert d.post_synaptic_density[0].segment_id == 0
    assert d.post_synaptic_density[0].offset == pytest.approx(0.8525)

    d = DendriticSpine(Path(spine_path))
    assert d.cell_family == CellFamily.SPINE

    with pytest.raises(RawDataError):
        DendriticSpine(Path(_path, 'simple.swc'))

    with pytest.raises(RawDataError):
        DendriticSpine(Path(_path, 'h5/v1/simple.h5'))


def test_dendritic_spine_add_root_section():
    # PointLevel + diameters parameters for append_root_section
    spine_morph = morphio.mut.DendriticSpine()
    spine_morph.append_root_section(
        morphio.PointLevel([[1.1, 2.1, 3.1], [4.1, 5.1, 6.1]],
                           [2.1, 3.1]),
        morphio.SectionType.spine_head)

    assert spine_morph.root_sections[0].type == morphio.SectionType.spine_head

    assert_array_almost_equal(spine_morph.root_sections[0].points,
                                  [[1.1, 2.1, 3.1], [4.1, 5.1, 6.1]])
    assert_array_almost_equal(spine_morph.root_sections[0].diameters,
                                  [2.1, 3.1])

    # Section parameter for append_root_section
    spine_morph = morphio.mut.DendriticSpine()
    spine_path = os.path.join(_path, 'h5/v1/simple-dendritric-spine.h5')
    input_spine = morphio.DendriticSpine(spine_path)
    spine_morph.append_root_section(input_spine.root_sections[0])
    assert_array_almost_equal(spine_morph.root_sections[0].points,
                                  [[0., 5., 0.], [2.4, 9.1, 0.],
                                   [0., 13.2, 0.]])
    assert spine_morph.root_sections[0].type == morphio.SectionType.spine_head
    assert_array_almost_equal(spine_morph.root_sections[0].diameters,
                                  [0.1, 0.2, 0.15])
