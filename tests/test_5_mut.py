# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
import numpy as np
from numpy import testing as npt
from pathlib import Path

import morphio
from morphio import CellFamily, IterType, MitochondriaPointLevel
from morphio import Morphology as ImmutableMorphology
from morphio import PointLevel, SectionType, ostream_redirect
from morphio.mut import GlialCell, Morphology, DendriticSpine
import pytest
from numpy.testing import assert_array_equal

from utils import captured_output

DATA_DIR = Path(__file__).parent / "data"

SIMPLE = Morphology(Path(DATA_DIR, "simple.swc"))


def test_heterogeneous_sections():

    morph = morphio.mut.Morphology(Path(DATA_DIR, "simple-heterogeneous-neurite.swc"))

    for root_section in morph.root_sections:

        assert root_section.is_heterogeneous()
        assert root_section.is_heterogeneous(True)
        assert root_section.is_heterogeneous(downstream=True)
        assert not root_section.is_heterogeneous(False)
        assert not root_section.is_heterogeneous(downstream=False)

        for section in root_section.children:
            assert not section.is_heterogeneous(downstream=True)
            assert section.is_heterogeneous(downstream=False)

    # check a homogeneous morphology
    for section in SIMPLE.iter():
        assert not section.is_heterogeneous(downstream=True)
        assert not section.is_heterogeneous(downstream=False)


def test_point_level():
    a = PointLevel([[1, 2, 3]], [2], [])
    assert a.points == [[1, 2, 3]]
    assert a.diameters == [2]

    with pytest.raises(morphio.SectionBuilderError,
                       match='Point vector have size: 2 while Diameter vector has size: 1'):
        PointLevel([[1, 2, 3],
                    [1, 2, 3]],
                   [2],
                   [])

    with pytest.raises(morphio.SectionBuilderError,
                       match='Point vector have size: 2 while Perimeter vector has size: 1'):
        PointLevel([[1, 2, 3],
                    [1, 2, 3]],
                   [2, 3],
                   [4])


def test_connectivity():
    cells = {
        'asc': Morphology(Path(DATA_DIR, "simple.asc")),
        'swc': Morphology(Path(DATA_DIR, "simple.swc")),
        'h5': Morphology(Path(DATA_DIR, "h5/v1/simple.h5")),
    }

    for cell in cells:
        assert cells[cell].connectivity == {-1: [0, 3], 0: [1, 2], 3: [4, 5]}


def test_empty_neurite():
    m = Morphology()
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            root = m.append_root_section(PointLevel(), SectionType.axon)
            assert (err.getvalue().strip() ==
                         'Warning: appending empty section with id: 0')

    assert len(m.root_sections) == 1
    assert (m.root_sections[0].type ==
                 SectionType.axon)

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            root.append_section(PointLevel(), SectionType.axon)
            assert (err.getvalue().strip() ==
                         'Warning: appending empty section with id: 1')

def test_single_neurite():
    m = Morphology()
    m.append_root_section(PointLevel([[1, 2, 3]], [2], [20]),
                          SectionType.axon)

    assert_array_equal(m.root_sections[0].points,
                       [[1, 2, 3]])
    assert (m.root_sections[0].diameters ==
                 [2])
    assert (m.root_sections[0].perimeters ==
                 [20])

    m.root_sections[0].points = [[10, 20, 30]]
    assert_array_equal(m.root_sections[0].points,
                       [[10, 20, 30]],
                       'Points array should have been mutated')

    m.root_sections[0].diameters = [7]

    assert (m.root_sections[0].diameters ==
                 [7]), 'Diameter array should have been mutated'

    m.root_sections[0].perimeters = [27]
    assert (m.root_sections[0].perimeters ==
                 [27]), 'Perimeter array should have been mutated'


def test_child_section():
    m = Morphology()
    section = m.append_root_section(PointLevel([[1, 2, 3]], [2], [20]),
                                    SectionType.axon)

    assert section.is_root

    section.append_section(PointLevel([[1, 2, 3], [4, 5, 6]],
                                      [2, 3],
                                      [20, 30]))

    children = m.root_sections[0].children

    assert (len(children) ==
                 1)

    assert_array_equal(children[0].points,
                       [[1, 2, 3], [4, 5, 6]])
    assert_array_equal(children[0].diameters,
                       [2, 3])
    assert_array_equal(children[0].perimeters,
                       [20, 30])

    children[0].points = [[7, 8, 9], [10, 11, 12]]

    assert_array_equal(children[0].points,
                       [[7, 8, 9], [10, 11, 12]])


def test_append_no_duplicate():
    m = Morphology()

    section = m.append_root_section(PointLevel([[1, 2, 3], [4, 5, 6]],
                                               [2, 2],
                                               [20, 20]),
                                    SectionType.axon)
    assert section.id == 0

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):

            section.append_section(
                             PointLevel([[400, 5, 6], [7, 8, 9]],
                                        [2, 3],
                                        [20, 30]))
            assert (err.getvalue().strip() ==
                         'Warning: while appending section: 1 to parent: 0\n'
                         'The section first point should be parent section last point: \n'
                         '        : X Y Z Diameter\n'
                         'parent last point :[4.000000, 5.000000, 6.000000, 2.000000]\n'
                         'child first point :[400.000000, 5.000000, 6.000000, 2.000000]')


def test_mut_copy_ctor():
    simple = Morphology(Path(DATA_DIR, "simple.swc"))
    assert ([sec.id for sec in simple.iter()] ==
                 [0, 1, 2, 3, 4, 5])
    copy = Morphology(simple)
    copy.append_root_section(PointLevel([[1, 2, 3], [4, 5, 6]],
                                        [2, 2],
                                        [20, 20]),
                             SectionType.axon)

    # test that first object has not been mutated
    assert ([sec.id for sec in simple.iter()] ==
                 [0, 1, 2, 3, 4, 5])

    assert ([sec.id for sec in copy.iter()] ==
                 [0, 1, 2, 3, 4, 5, 6])



def test_build_read_only():
    m = Morphology()
    m.soma.points = [[-1, -2, -3]]
    m.soma.diameters = [-4]

    section = m.append_root_section(PointLevel([[1, 2, 3], [4, 5, 6]],
                                               [2, 2],
                                               [20, 20]),
                                    SectionType.axon)

    section.append_section(PointLevel([[4, 5, 6], [7, 8, 9]],
                                      [2, 3],
                                      [20, 30]))

    section.append_section(PointLevel([[4, 5, 6], [10, 11, 12]],
                                      [2, 2],
                                      [20, 20]))

    immutable_morphology = ImmutableMorphology(m)

    sections = list(immutable_morphology.iter())
    assert len(sections) == 3

    assert_array_equal(immutable_morphology.soma.points,
                       [[-1, - 2, -3]])
    assert_array_equal(immutable_morphology.soma.diameters,
                       [-4])

    assert_array_equal(immutable_morphology.section(0).points,
                       [[1, 2, 3], [4, 5, 6]])
    assert_array_equal(immutable_morphology.section(0).diameters,
                       [2, 2])
    assert_array_equal(immutable_morphology.section(0).perimeters,
                       [20, 20])

    assert len(immutable_morphology.section(0).children) == 2

    child = immutable_morphology.section(0).children[0]
    assert_array_equal(child.points,
                       [[4, 5, 6], [7, 8, 9]])
    assert_array_equal(child.diameters,
                       [2, 3])
    assert_array_equal(child.perimeters,
                       [20, 30])

    same_child = immutable_morphology.section(1)
    assert_array_equal(same_child.points,
                       [[4, 5, 6], [7, 8, 9]])
    assert_array_equal(same_child.diameters,
                       [2, 3])
    assert_array_equal(same_child.perimeters,
                       [20, 30])


def test_mutable_immutable_equivalence():
    morpho = ImmutableMorphology(Path(DATA_DIR, "simple.swc"))
    assert_array_equal(morpho.points, morpho.as_mutable().as_immutable().points)


def test_mitochondria_read():
    """Read a H5 file with a mitochondria"""
    morpho = Morphology(DATA_DIR / "h5/v1/mitochondria.h5")
    mito = morpho.mitochondria
    assert len(mito.root_sections) == 2

    mitochondria = mito.root_sections

    assert_array_equal(mitochondria[0].diameters,
                       [10, 20])
    assert_array_equal(mitochondria[0].relative_path_lengths,
                       np.array([0.5, 0.6], dtype=np.float32))
    assert_array_equal(mitochondria[0].neurite_section_ids,
                       np.array([0., 0.], dtype=np.float32))

    assert len(mito.children(mito.root_sections[0])) == 1

    assert (mito.parent(mito.children(mitochondria[0])[0]) ==
                 mitochondria[0])

    assert_array_equal(mitochondria[1].diameters,
                       [5, 6, 7, 8])
    assert_array_equal(mitochondria[1].relative_path_lengths,
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))
    assert_array_equal(mitochondria[1].neurite_section_ids,
                       np.array([0, 1, 1, 2], dtype=np.float32))

    assert len(mito.children(mito.root_sections[1])) == 0


def test_sections_are_not_dereferenced():
    """There used to be a bug where if you would call:
    mitochondria.sections, that would dereference all section pointers
    if mitochondria.sections was not kept in a variable"""
    morpho = Morphology(Path(DATA_DIR, "h5/v1/mitochondria.h5"))

    # This lines used to cause a bug
    morpho.mitochondria.sections  # pylint: disable=pointless-statement

    assert all(section is not None for section in morpho.mitochondria.sections.values())


def test_append_mutable_section():
    morpho = Morphology()
    second_children_first_root = SIMPLE.root_sections[0].children[1]

    morpho.append_root_section(second_children_first_root)
    assert len(morpho.root_sections) == 1

    assert_array_equal(morpho.root_sections[0].points,
                       second_children_first_root.points)


def test_mitochondria():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [1, 1, 1]]
    morpho.soma.diameters = [1, 1]

    morpho.append_root_section(PointLevel([[2, 2, 2], [3, 3, 3]],
                                          [4, 4],
                                          [5, 5]),
                               SectionType.axon)

    mito = morpho.mitochondria
    first_mito_id = mito.append_root_section(MitochondriaPointLevel([0, 0], [0.5, 0.6],
                                                                    [10, 20]))

    first_child = first_mito_id.append_section(
                                      MitochondriaPointLevel([3, 4, 4, 5],
                                                             [0.6, 0.7, 0.8, 0.9],
                                                             [20, 30, 40, 50]))

    second_mito_id = mito.append_root_section(
                                         MitochondriaPointLevel([0, 1, 1, 2],
                                                                [0.6, 0.7, 0.8, 0.9],
                                                                [5, 6, 7, 8]))

    assert mito.is_root(first_mito_id)
    assert mito.children(first_mito_id) == [first_child]
    assert mito.parent(first_child) == first_mito_id
    assert mito.root_sections == [first_mito_id, second_mito_id]

    assert_array_equal(first_child.diameters,
                       [20, 30, 40, 50])
    assert_array_equal(first_child.neurite_section_ids,
                       [3, 4, 4, 5])

    assert_array_equal(np.array(first_child.relative_path_lengths, dtype=np.float32),
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))


def test_iterators():
    assert_array_equal([sec.id for sec in SIMPLE.section(5).iter(IterType.upstream)],
                       [5, 3])

    assert_array_equal([sec.id for sec in SIMPLE.section(0).iter(IterType.depth_first)],
                       [0, 1, 2])

    assert_array_equal([sec.id for sec in SIMPLE.section(0).iter(IterType.breadth_first)],
                       [0, 1, 2])

    assert_array_equal([sec.id for sec in SIMPLE.iter()],
                       [0, 1, 2, 3, 4, 5])

    assert_array_equal([sec.id for sec in SIMPLE.iter(IterType.depth_first)],
                       [0, 1, 2, 3, 4, 5])

    assert_array_equal([sec.id for sec in SIMPLE.iter(IterType.breadth_first)],
                       [0, 3, 1, 2, 4, 5])

    neuron = Morphology(Path(DATA_DIR, "iterators.asc"))
    root = neuron.root_sections[0]
    assert_array_equal([section.id for section in root.iter(IterType.depth_first)],
                       [0, 1, 2, 3, 4, 5, 6])
    assert_array_equal([section.id for section in root.iter(IterType.breadth_first)],
                       [0, 1, 4, 2, 3, 5, 6])

    assert_array_equal([section.id for section in neuron.iter(IterType.breadth_first)],
                       [0, 7, 1, 4, 8, 9, 2, 3, 5, 6])

def test_non_C_nparray():
    m = Morphology(Path(DATA_DIR, "simple.swc"))
    section = m.root_sections[0]
    points = np.array([[1, 2, 3], [4, 5, 6]])
    section.points = points
    assert_array_equal(section.points, points)

    with pytest.raises(morphio.MorphioError,
                       match=r'Wrong array shape. Expected: \(X, 3\), got: \(3, 2\)'):
        section.points = points.T

    non_standard_stride = np.asfortranarray(points)
    section.points = non_standard_stride
    assert_array_equal(section.points, points)


def test_annotation(tmp_path):
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            content = ("""((Dendrite)
                            (3 -4 0 2)
                            (3 -6 0 2)
                            (3 -8 0 2)
                            (3 -10 0 2)
                            (
                              (3 -10 0 2)
                              (0 -10 0 2)
                              (-3 -10 0 2)
                              |       ; <-- empty sibling but still works !
                             )
                            )
                       """)
            cell = ImmutableMorphology(content, extension='asc').as_mutable()
            cell.remove_unifurcations()

    for n in (cell, cell.as_immutable(), cell.as_immutable().as_mutable()):
        assert len(n.annotations) == 1
        annotation = n.annotations[0]
        assert annotation.type == morphio.AnnotationType.single_child


def test_empty_sibling(tmp_path):
    '''The empty sibling will be removed and the single child will be merged
    with its parent'''
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            content = ('''((Dendrite)
                            (3 -4 0 10)
                            (3 -6 0 9)
                            (3 -8 0 8)
                            (3 -10 0 7)
                            (
                              (3 -10 0 6)
                              (0 -10 0 5)
                              (-3 -10 0 4)
                              |       ; <-- empty sibling but still works !
                             )
                            )
                       ''')
            n = ImmutableMorphology(content, extension='asc').as_mutable()
            n.remove_unifurcations()
            assert 'is the only child of section: 0' in err.getvalue()
            assert 'It will be merged with the parent section' in err.getvalue()

    assert len(n.root_sections) == 1
    assert_array_equal(n.root_sections[0].points,
                       np.array([[3, -4, 0],
                                 [3, -6, 0],
                                 [3, -8, 0],
                                 [3, -10, 0],
                                 [0, -10, 0],
                                 [-3, -10, 0]],
                                dtype=np.float32))
    assert_array_equal(n.root_sections[0].diameters,
                       np.array([10, 9, 8, 7, 5, 4], dtype=np.float32))

    assert len(n.annotations) == 1
    annotation = n.annotations[0]
    assert annotation.type == morphio.AnnotationType.single_child
    assert annotation.line_number == -1
    assert_array_equal(annotation.points, [[3, -10, 0], [0, -10, 0], [-3, -10, 0]])
    assert_array_equal(annotation.diameters, [6, 5, 4])


def test_nested_single_child():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            n = Morphology(DATA_DIR / 'nested_single_children.asc')
            n.remove_unifurcations()
    assert_array_equal(n.root_sections[0].points,
                       [[0., 0., 0.],
                        [0., 0., 1.],
                        [0., 0., 2.],
                        [0., 0., 3.],
                        [0., 0., 4.]])
    assert_array_equal(n.root_sections[0].diameters, np.array([8, 7, 6, 5, 4], dtype=np.float32))


def test_section___str__():
    assert (str(SIMPLE.root_sections[0]) ==
                 'Section(id=0, points=[(0 0 0),..., (0 5 0)])')


def test_from_pathlib():
    neuron = Morphology(DATA_DIR / "simple.asc")
    assert len(neuron.root_sections) == 2


def test_endoplasmic_reticulum(tmp_path):
    neuron = Morphology(DATA_DIR / "simple.asc")
    reticulum = neuron.endoplasmic_reticulum
    assert len(reticulum.section_indices) == 0
    assert len(reticulum.volumes) == 0
    assert len(reticulum.surface_areas) == 0
    assert len(reticulum.filament_counts) == 0

    reticulum.section_indices = [1, 1]
    reticulum.volumes = [2, 2]
    reticulum.surface_areas = [3, 3]
    reticulum.filament_counts = [4, 4]

    assert_array_equal(reticulum.section_indices, [1, 1])
    assert_array_equal(reticulum.volumes, [2, 2])
    assert_array_equal(reticulum.surface_areas, [3, 3])
    assert_array_equal(reticulum.filament_counts, [4, 4])

    path = tmp_path / 'with-reticulum.h5'
    neuron.write(path)
    neuron = Morphology(path)

    reticulum = neuron.endoplasmic_reticulum
    assert_array_equal(reticulum.section_indices, [1, 1])
    assert_array_equal(reticulum.volumes, [2, 2])
    assert_array_equal(reticulum.surface_areas, [3, 3])
    assert_array_equal(reticulum.filament_counts, [4, 4])


def test_remove_unifurcations():
    m = Morphology()
    section = m.append_root_section(PointLevel([[1, 0, 0],
                                                [2, 0, 0]], [2, 2], [20, 20]),
                                    SectionType.axon)
    section.append_section(PointLevel([[2, 0, 0],
                                       [3, 0, 0]], [2, 2], [20, 20]))
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            m.remove_unifurcations()
            assert len(list(m.iter())) == 1
            assert (err.getvalue().strip() ==
                         'Warning: section 1 is the only child of section: 0\nIt will be merged '
                         'with the parent section')

    # Checking that remove_unifurcations() issues a warning on missing duplicate
    m = Morphology()
    section = m.append_root_section(PointLevel([[1, 0, 0],
                                                [2, 0, 0]], [2, 2], [20, 20]),
                                    SectionType.axon)
    section.append_section(PointLevel([[2, 0, 0],
                                       [3, 0, 0]], [2, 2], [20, 20]))
    with captured_output() as (_, err):
        with ostream_redirect():
            section.append_section(PointLevel([[2, 1, 0],
                                               [2, 0, 0]], [2, 2], [20, 20]))
    with captured_output() as (_, err):
        with ostream_redirect():
            m.remove_unifurcations()
            assert err.getvalue().strip() == (
                'Warning: while appending section: 2 to parent: 0\n'
                'The section first point should be parent section last point: \n'
                '        : X Y Z Diameter\n'
                'parent last point :[2.000000, 0.000000, 0.000000, 2.000000]\n'
                'child first point :[2.000000, 1.000000, 0.000000, 2.000000]')

def test_remove_rootsection():
    morpho = Morphology(DATA_DIR / 'single_point_root.asc')
    assert len(morpho.root_sections) == 1
    to_remove = []
    for root in morpho.root_sections:
        if len(root.points) == 1:
            to_remove.append(root)
    for root in to_remove:
        morpho.delete_section(root, False)
    assert len(morpho.root_sections) == 2


def test_remove_rootsection_in_loop():
    morpho = Morphology(DATA_DIR / 'single_point_root.asc')
    assert len(morpho.root_sections) == 1
    for root in morpho.root_sections:
        if len(root.points) == 1:
            morpho.delete_section(root, False)
    assert len(morpho.root_sections) == 2


def test_glia():
    g = GlialCell()
    assert g.cell_family == CellFamily.GLIA

    g = GlialCell(Path(DATA_DIR, 'astrocyte.h5'))
    assert g.cell_family == CellFamily.GLIA

    g = GlialCell(DATA_DIR / 'astrocyte.h5')
    assert g.cell_family == CellFamily.GLIA

    with pytest.raises(morphio.RawDataError):
        GlialCell(DATA_DIR / 'simple.swc')
    with pytest.raises(morphio.RawDataError):
        GlialCell(DATA_DIR / 'h5/v1/simple.h5')


def test_glia_round_trip(tmp_path):
    g = GlialCell(Path(DATA_DIR, 'astrocyte.h5'))
    filename = tmp_path / 'glial-cell.h5'
    g.write(filename)
    g2 = GlialCell(filename)
    assert len(g.sections) == len(g2.sections)

def test_dendritic_spine():
    d = DendriticSpine()
    assert d.cell_family == CellFamily.SPINE

    d = DendriticSpine(DATA_DIR / "h5/v1/" / 'simple-dendritric-spine.h5')
    assert d.cell_family == CellFamily.SPINE

    psd = d.post_synaptic_density
    assert len(psd) == 2
    assert psd[0].section_id == 1
    assert psd[0].segment_id == 0
    assert psd[0].offset == pytest.approx(0.8525)

    with pytest.raises(morphio.RawDataError):
        DendriticSpine(DATA_DIR / 'simple.swc')

    with pytest.raises(morphio.RawDataError):
        DendriticSpine(DATA_DIR / 'h5/v1/simple.h5')


def test_dendritic_spine_round_trip(tmp_path):
    d = DendriticSpine(DATA_DIR / "h5/v1/" / 'simple-dendritric-spine.h5')
    filename = tmp_path / 'test-dendritic-spine.h5'
    d.write(filename)
    d2 = DendriticSpine(filename)

    assert len(d.sections) == len(d2.sections)
    assert len(d.post_synaptic_density) == len(d2.post_synaptic_density)

    d2 = DendriticSpine(filename)
    d2.post_synaptic_density = (d2.post_synaptic_density +
                                d2.post_synaptic_density +
                                [morphio.PostSynapticDensity(1, 2, 3.3)])
    d2.write(filename)

    d3 = DendriticSpine(filename)
    assert len(d2.sections) == len(d3.sections)
    assert len(d2.post_synaptic_density) == len(d3.post_synaptic_density)

    psd = d3.post_synaptic_density
    assert len(psd) == 2 + 2 + 1
    assert psd[-1].section_id == 1
    assert psd[-1].segment_id == 2
    assert psd[-1].offset == pytest.approx(3.3)


def test_dendritic_spine_round_trip_empty_postsynaptic_density(tmp_path):
    d = DendriticSpine(DATA_DIR / "h5/v1/" / 'simple-dendritric-spine.h5')
    assert d.cell_family == CellFamily.SPINE

    d.post_synaptic_density = []

    filename = tmp_path / 'test-empty-spine.h5'
    d.write(filename)

    d2 = DendriticSpine(filename)
    assert d.post_synaptic_density == d2.post_synaptic_density


def _get_section():
    """This is used so that the reference to m is destroyed."""
    m = Morphology(DATA_DIR / 'simple.swc')
    s = m.root_sections[0]
    return s


def test_lifetime_destroyed_morphology():
    """Accessing topological info after a Morphology has been destroyed should raise."""
    m = Morphology(DATA_DIR / 'simple.swc')
    s = m.root_sections[0]

    del m  # ~mut.Morphology() called

    with pytest.raises(RuntimeError):
        s.children


def test_lifetime_access_properties_no_morphology():
    """Accessing topological info after a Morphology has been destroyed should raise but
    "self properties" should be available.
    """
    section = _get_section()
    assert_array_equal(section.points,
                       np.array([[0., 0., 0.],
                                 [0., 5., 0.]], dtype=np.float32))

    with pytest.raises(RuntimeError):
        section.children


def test_lifetime_copy_single_section():
    """Copying a single section from a destroyed morphology works because it
    does not use any topological information"""
    section = _get_section()

    # Proof that the morphology has really been destroyed
    with pytest.raises(RuntimeError):
        section.children

    morph = Morphology()
    morph.append_root_section(section)
    del section
    assert len(morph.root_sections) == 1
    assert_array_equal(morph.root_sections[0].points,
                       np.array([[0., 0., 0.],
                                 [0., 5., 0.]], dtype=np.float32))


def test_lifetime_iteration_fails_with_orphan_section():
    section = _get_section()
    for iter_type in IterType.depth_first, IterType.breadth_first, IterType.upstream:
        with pytest.raises(RuntimeError):
            section.iter(iter_type)


def _assert_sections_equal(morph1, morph2, section_ids):

    for section_id in section_ids:

        s1 = morph1.section(section_id)
        s2 = morph2.section(section_id)

        assert s1.type == s2.type
        npt.assert_array_equal(s1.points, s2.points)
        npt.assert_array_equal(s1.diameters, s2.diameters)


def test_delete_section__maintain_children_order_non_root_section():
    string = """
    ("CellBody"
      (Color Red)
      (CellBody)
      ( 0.1  0.1 0.0 0.1)
      ( 0.1 -0.1 0.0 0.1)
    )

    ( (Color Cyan)
      (Axon)
      (0.0  0.0 0.0 2.0) ; section 0
      (0.0 -4.0 1.0 2.0)
      (
        (0.0 -4.0 1.0 4.0) ; section 1
        (0.0 -4.0 2.0 4.0)
        (0.0 -4.0 3.0 4.0)
        (
            (6.0 -4.0 0.0 4.0) ; section 2
            (7.0 -5.0 0.0 4.0)
        |
            (6.0 -4.0 0.0 4.0) ; section 3
            (8.0 -4.0 0.0 4.0)
        )
      |
        ( 0.0 -4.0 1.0 4.0) ; section 4
        (-5.0 -4.0 0.0 4.0)
      )
    )
    """
    morph = ImmutableMorphology(string, "asc").as_mutable()

    axon = morph.root_sections[0]
    to_be_removed_section = axon.children[0]
    morph.delete_section(to_be_removed_section, recursive=False)

    assert [s.id for s in morph.iter()] == [0, 2, 3, 4]

    # a trifurcation is made
    assert [s.id for s in axon.children] == [2, 3, 4]

    # ensure nothing has been added to the leaves
    for child in axon.children:
        assert child.children == []

    # check that data has not been messed up
    original = ImmutableMorphology(string, "asc").as_mutable()
    _assert_sections_equal(morph, original, [0, 2, 3, 4])


def test_delete_section__maintain_children_order_non_root_section_multiple():
    string = """
    ("CellBody"
      (Color Red)
      (CellBody)
      ( 0.1  0.1 0.0 0.1)
      ( 0.1 -0.1 0.0 0.1)
    )

    ( (Color Cyan)
      (Axon)
      (0.0  0.0 0.0 2.0) ; section 0
      (0.0 -4.0 1.0 2.0)
      (
        (0.0 -4.0 1.0 4.0) ; section 1
        (0.0 -4.0 2.0 4.0)
        (0.0 -4.0 3.0 4.0)
        (
            (6.0 -4.0 0.0 4.0) ; section 2
            (7.0 -5.0 0.0 4.0)
            (
                (8.0 -4.0 0.0 4.0) ; section 3
                (8.0 -5.0 0.0 4.0)
            |
                (8.0 -4.0 0.0 4.0) ; section 4
                (8.0 -5.0 0.0 4.0)
            )
        |
            (6.0 -4.0 0.0 4.0) ; section 5
            (8.0 -4.0 0.0 4.0)
        )
      |
        ( 0.0 -4.0 1.0 4.0) ; section 6
        (-5.0 -4.0 0.0 4.0)
        (
            (3.0 2.0 1.0 4.0) ; section 7
            (3.0 2.0 0.0 4.0)
        |
            (2.0 3.0 1.0 4.0) ; section 8
            (2.0 3.0 0.0 4.0)
        )
      )
    )
    """
    morph = ImmutableMorphology(string, "asc").as_mutable()

    # for reference and sanity
    assert([s.id for s in morph.iter(iter_type=morphio.IterType.depth_first)] ==
           [0, 1, 2, 3, 4, 5, 6, 7, 8])
    assert([s.id for s in morph.iter(iter_type=morphio.IterType.breadth_first)] ==
           [0, 1, 6, 2, 5, 7, 8, 3, 4])

    morph.delete_section(morph.section(0), recursive=False)
    morph.delete_section(morph.section(1), recursive=False)
    morph.delete_section(morph.section(4), recursive=False)
    morph.delete_section(morph.section(7), recursive=False)

    assert [s.id for s in morph.iter()] == [2, 3, 5, 6, 8]
    #assert [s.id for s in morph.iter(iter_type=morphio.IterType.breadth_first)] == [6, 2, 5, 8, 3]
    assert [s.id for s in morph.root_sections] == [2, 5, 6]

    # made unifurcations
    assert [s.id for s in morph.section(2).children] == [3]
    assert [s.id for s in morph.section(6).children] == [8]

    # ensure nothing has been added to the leaves
    for leaf_id in [3, 5, 8]:
        assert morph.section(leaf_id).children == []

    # check that data has not been messed up
    original = ImmutableMorphology(string, "asc").as_mutable()
    _assert_sections_equal(morph, original, [2, 3, 5, 6, 8])


def test_delete_section__maintain_children_order_root_section():
    string = """
    ("CellBody"
      (Color Red)
      (CellBody)
      ( 0.1  0.1 0.0 0.1)
      ( 0.1 -0.1 0.0 0.1)
    )

    ( (Color Cyan)
      (Axon)
      (0.0  0.0 0.0 2.0) ; section 0
      (0.0 -4.0 1.0 2.0)
      (
        (0.0 -4.0 1.0 4.0) ; section 1
        (0.0 -4.0 2.0 4.0)
        (0.0 -4.0 3.0 4.0)
        (
            (6.0 -4.0 0.0 4.0) ; section 2
            (7.0 -5.0 0.0 4.0)
        |
            (6.0 -4.0 0.0 4.0) ; section 3
            (8.0 -4.0 0.0 4.0)
        )
      |
        ( 0.0 -4.0 1.0 4.0) ; section 4
        (-5.0 -4.0 0.0 4.0)
      )
    )

    ( (Color Cyan)
      (Dendrite)
      (1.0  1.0 1.0 2.0) ; section 5
      (1.0  1.0 2.0 2.0)
    )
    """
    morph = ImmutableMorphology(string, "asc").as_mutable()

    root = morph.root_sections[0]
    morph.delete_section(root, recursive=False)

    assert [s.id for s in morph.iter()] == [1, 2, 3, 4, 5]

    # the children of root 0 are put in its place
    assert [s.id for s in morph.root_sections] == [1, 4, 5]
    assert [s.id for s in morph.root_sections[0].children] == [2, 3]
    assert [s.id for s in morph.root_sections[1].children] == []
    assert [s.id for s in morph.root_sections[2].children] == []

    # check that nothing has been added to the leaves
    for section_id in [2, 3, 4, 5]:
        assert morph.section(section_id).children == []

    # check that data has not been messed up
    original = ImmutableMorphology(string, "asc").as_mutable()
    _assert_sections_equal(morph, original, [1, 2, 3, 4, 5])


def test_delete_section__unifurcation_bridge():
    pl = morphio.PointLevel(
        [[1., 2., 3.], [1., 2., 3.]],
        [0.1, 0.1],
    )

    basal = morphio.SectionType.basal_dendrite
    axon =  morphio.SectionType.axon

    morph = morphio.mut.Morphology()

    s0 = morph.append_root_section(pl, basal) # 0
    s1 = morph.append_root_section(pl, axon) # 1
    s2 = morph.append_root_section(pl, basal) # 2

    # a unifurcation, only one child
    s3 = s1.append_section(pl, axon) # 3
    s4 = s3.append_section(pl, axon) # 4

    s4.append_section(pl, axon) # 5
    s4.append_section(pl, axon) # 6

    assert [s.id for s in morph.iter()] == [0, 1, 3, 4, 5, 6, 2]

    morph.delete_section(s3, recursive=False)

    assert [s.id for s in morph.iter()] == [0, 1, 4, 5, 6, 2]
    assert [s.parent.id for s in s1.children] == [1]
    assert [s.id for s in s1.children] == [4]

    morph.delete_section(s4, recursive=False)

    assert [s.id for s in morph.iter()] == [0, 1, 5, 6, 2]
    assert [s.parent.id for s in s1.children] == [1, 1]
    assert [s.id for s in s1.children] == [5, 6]


def test_delete_section__remove_all_children():

    pl = morphio.PointLevel(
        [[1., 2., 3.], [1., 2., 3.]],
        [0.1, 0.1],
    )

    basal = morphio.SectionType.basal_dendrite

    morph = morphio.mut.Morphology()

    s0 = morph.append_root_section(pl, basal) # 0
    s1 = s0.append_section(pl, basal) # 1
    s2 = s0.append_section(pl, basal) # 2

    morph.delete_section(s1, recursive=False)
    morph.delete_section(s2, recursive=False)

    assert [s.id for s in morph.iter()] == [0]
    assert not s0.children

    s3 = s0.append_section(pl, basal) # 3
    s4 = s0.append_section(pl, basal) # 4

    assert [s.id for s in morph.iter()] == [0, 3, 4]
    assert [s.id for s in s0.children] == [3, 4]


def test_delete_section__append_delete():

    pl = morphio.PointLevel(
        [[1., 2., 3.], [1., 2., 3.]],
        [0.1, 0.1],
    )

    basal = morphio.SectionType.basal_dendrite
    axon =  morphio.SectionType.axon

    morph = morphio.mut.Morphology()

    s0 = morph.append_root_section(pl, basal) # 0
    s1 = morph.append_root_section(pl, axon) # 1
    s2 = s1.append_section(pl, axon) # 2

    s3 = s1.append_section(pl, axon) # 3

    s4 = s3.append_section(pl, axon) # 4
    s5 = s3.append_section(pl, axon) # 5
    s6 = s3.append_section(pl, axon) # 6

    morph.delete_section(s4, recursive=False)
    s3.append_section(pl, axon) # 7
    morph.delete_section(s3, recursive=False)

    assert [s.id for s in morph.iter()] == [0, 1, 2, 5, 6, 7]

    assert [s.parent.id for s in s1.children] == [1, 1, 1, 1]
    assert [s.id for s in s1.children] == [2, 5, 6, 7]


def test_delete_section__traversals():
    string = """
    ("CellBody"
      (Color Red)
      (CellBody)
      ( 0.1  0.1 0.0 0.1)
      ( 0.1 -0.1 0.0 0.1)
    )

    ( (Color Cyan)
      (Axon)
      (0.0  0.0 0.0 2.0) ; section 0
      (0.0 -4.0 1.0 2.0)
      (
        (0.0 -4.0 1.0 4.0) ; section 1
        (0.0 -4.0 2.0 4.0)
        (0.0 -4.0 3.0 4.0)
        (
            (6.0 -4.0 0.0 4.0) ; section 2
            (7.0 -5.0 0.0 4.0)
        |
            (6.0 -4.0 0.0 4.0) ; section 3
            (8.0 -4.0 0.0 4.0)
        )
      |
        ( 0.0 -4.0 1.0 4.0) ; section 4
        (-5.0 -4.0 0.0 4.0)
      )
    )

    ( (Color Cyan)
      (Dendrite)
      (1.0  1.0 1.0 2.0) ; section 5
      (1.0  1.0 2.0 2.0)
    )
    """
    morph = ImmutableMorphology(string, "asc").as_mutable()

    assert [s.id for s in morph.iter(morphio.IterType.depth_first)] == [0, 1, 2, 3, 4, 5]
    assert [s.id for s in morph.iter(morphio.IterType.breadth_first)] == [0, 5, 1, 4, 2, 3]
    assert [s.id for s in morph.section(3).iter(morphio.IterType.upstream)] == [3, 1, 0]

    morph.delete_section(morph.section(1), recursive=False)

    assert [s.id for s in morph.iter(morphio.IterType.depth_first)] == [0, 2, 3, 4, 5]
    assert [s.id for s in morph.iter(morphio.IterType.breadth_first)] == [0, 5, 2, 3, 4]
    assert [s.id for s in morph.section(3).iter(morphio.IterType.upstream)] == [3, 0]

    morph.delete_section(morph.section(3), recursive=False)

    assert [s.id for s in morph.iter(morphio.IterType.depth_first)] == [0, 2, 4, 5]
    assert [s.id for s in morph.iter(morphio.IterType.breadth_first)] == [0, 5, 2, 4]

    morph.delete_section(morph.section(0), recursive=False)

    assert [s.id for s in morph.iter(morphio.IterType.depth_first)] == [2, 4, 5]
    assert [s.id for s in morph.iter(morphio.IterType.breadth_first)] == [2, 4, 5]

    morph.delete_section(morph.section(4), recursive=False)

    assert [s.id for s in morph.iter(morphio.IterType.depth_first)] == [2, 5]
    assert [s.id for s in morph.iter(morphio.IterType.breadth_first)] == [2, 5]
