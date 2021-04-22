from collections import OrderedDict

import numpy as np
from pathlib import Path
from tempfile import TemporaryDirectory

import morphio
from morphio import CellFamily, IterType, MitochondriaPointLevel, MorphioError
from morphio import Morphology as ImmutableMorphology
from morphio import PointLevel, RawDataError, SectionBuilderError, SectionType, ostream_redirect
from morphio.mut import GlialCell, Morphology
import pytest
from numpy.testing import assert_array_equal

from utils import assert_substring, captured_output, setup_tempdir, tmp_asc_file

DATA_DIR = Path(__file__).parent / "data"

SIMPLE = Morphology(Path(DATA_DIR, "simple.swc"))


def test_point_level():
    a = PointLevel([[1, 2, 3]], [2], [])
    assert a.points == [[1, 2, 3]]
    assert a.diameters == [2]

    with pytest.raises(SectionBuilderError, match='Point vector have size: 2 while Diameter vector has size: 1'):
        PointLevel([[1, 2, 3],
                    [1, 2, 3]],
                   [2],
                   [])

    with pytest.raises(SectionBuilderError, match='Point vector have size: 2 while Perimeter vector has size: 1'):
        PointLevel([[1, 2, 3],
                    [1, 2, 3]],
                   [2, 3],
                   [4])


def test_connectivity():
    cells = OrderedDict({
        'asc': Morphology(Path(DATA_DIR, "simple.asc")),
        'swc': Morphology(Path(DATA_DIR, "simple.swc")),
        'h5': Morphology(Path(DATA_DIR, "h5/v1/simple.h5")),
    })

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

    assert mito.is_root(first_mito_id) == True
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

    with pytest.raises(MorphioError, match=r'Wrong array shape. Expected: \(X, 3\), got: \(3, 2\)'):
        section.points = points.T

    non_standard_stride = np.asfortranarray(points)
    section.points = non_standard_stride
    assert_array_equal(section.points, points)

def test_annotation():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            with tmp_asc_file("""((Dendrite)
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
                 """) as tmp_file:
                cell = Morphology(tmp_file.name)
                cell.remove_unifurcations()

    for n in (cell, cell.as_immutable(), cell.as_immutable().as_mutable()):
        assert len(n.annotations) == 1
        annotation = n.annotations[0]
        assert annotation.type == morphio.AnnotationType.single_child

def test_empty_sibling():
    '''The empty sibling will be removed and the single child will be merged
    with its parent'''
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            with tmp_asc_file('''((Dendrite)
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
                 ''') as tmp_file:
                n = Morphology(tmp_file.name)
                n.remove_unifurcations()
                assert_substring('is the only child of section: 0',
                                 err.getvalue().strip())
                assert_substring('It will be merged with the parent section',
                                 err.getvalue().strip())

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


def test_endoplasmic_reticulum():
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

    with setup_tempdir('test-endoplasmic-reticulum') as folder:
        path = Path(folder, 'with-reticulum.h5')
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
            assert (err.getvalue().strip() ==
                         'Warning: while appending section: 2 to parent: 0\nThe section first point should be parent section last point: \n        : X Y Z Diameter\nparent last point :[2.000000, 0.000000, 0.000000, 2.000000]\nchild first point :[2.000000, 1.000000, 0.000000, 2.000000]')


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

    with pytest.raises(RawDataError):
        GlialCell(DATA_DIR / 'simple.swc')
    with pytest.raises(RawDataError):
        GlialCell(DATA_DIR / 'h5/v1/simple.h5')


def test_glia_round_trip():
    with TemporaryDirectory() as folder:
        g = GlialCell(Path(DATA_DIR, 'astrocyte.h5'))
        filename = Path(folder, 'glial-cell.h5')
        g.write(filename)
        g2 = GlialCell(filename)
        assert len(g.sections) == len(g2.sections)


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
