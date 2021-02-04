import os
from collections import OrderedDict

import numpy as np
from numpy.testing import assert_equal
from nose.tools import assert_dict_equal, assert_raises, ok_
from numpy.testing import assert_array_equal
from pathlib import Path
from tempfile import TemporaryDirectory

import morphio
from morphio import MitochondriaPointLevel, MorphioError, RawDataError
from morphio import Morphology as ImmutableMorphology
from morphio import (PointLevel, SectionBuilderError, SectionType,
                     IterType, ostream_redirect, CellFamily)
from morphio.mut import Morphology, GlialCell
from . utils import assert_substring, captured_output, tmp_asc_file, setup_tempdir

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")

SIMPLE = Morphology(os.path.join(_path, "simple.swc"))


def test_point_level():
    a = PointLevel([[1, 2, 3]], [2], [])
    assert_equal(a.points, [[1, 2, 3]])
    assert_equal(a.diameters, [2])

    with assert_raises(SectionBuilderError) as obj:
        a = PointLevel([[1, 2, 3],
                        [1, 2, 3]],
                       [2],
                       [])

    assert_substring("Point vector have size: 2 while Diameter vector has size: 1",
                     str(obj.exception))

    with assert_raises(SectionBuilderError) as obj:
        a = PointLevel([[1, 2, 3],
                        [1, 2, 3]],
                       [2, 3],
                       [4])

    assert_substring("Point vector have size: 2 while Perimeter vector has size: 1",
                     str(obj.exception))


def test_connectivity():
    cells = OrderedDict({
        'asc': Morphology(os.path.join(_path, "simple.asc")),
        'swc': Morphology(os.path.join(_path, "simple.swc")),
        'h5': Morphology(os.path.join(_path, "h5/v1/simple.h5")),
    })

    for cell in cells:
        assert_dict_equal(cells[cell].connectivity, {-1: [0, 3], 0: [1, 2], 3: [4, 5]})


def test_empty_neurite():
    m = Morphology()
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            root = m.append_root_section(PointLevel(), SectionType.axon)
            assert_equal(err.getvalue().strip(),
                         'Warning: appending empty section with id: 0')

    assert_equal(len(m.root_sections), 1)
    assert_equal(m.root_sections[0].type,
                 SectionType.axon)

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            root.append_section(PointLevel(), SectionType.axon)
            assert_equal(err.getvalue().strip(),
                         'Warning: appending empty section with id: 1')

def test_single_neurite():
    m = Morphology()
    m.append_root_section(PointLevel([[1, 2, 3]], [2], [20]),
                          SectionType.axon)

    assert_array_equal(m.root_sections[0].points,
                       [[1, 2, 3]])
    assert_equal(m.root_sections[0].diameters,
                 [2])
    assert_equal(m.root_sections[0].perimeters,
                 [20])

    m.root_sections[0].points = [[10, 20, 30]]
    assert_array_equal(m.root_sections[0].points,
                       [[10, 20, 30]],
                       'Points array should have been mutated')

    m.root_sections[0].diameters = [7]

    assert_equal(m.root_sections[0].diameters,
                 [7],
                 'Diameter array should have been mutated')

    m.root_sections[0].perimeters = [27]
    assert_equal(m.root_sections[0].perimeters,
                 [27],
                 'Perimeter array should have been mutated')


def test_child_section():
    m = Morphology()
    section = m.append_root_section(PointLevel([[1, 2, 3]], [2], [20]),
                                    SectionType.axon)

    ok_(section.is_root)

    section.append_section(PointLevel([[1, 2, 3], [4, 5, 6]],
                                      [2, 3],
                                      [20, 30]))

    children = m.root_sections[0].children

    assert_equal(len(children),
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
    assert_equal(section.id, 0)

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):

            section.append_section(
                             PointLevel([[400, 5, 6], [7, 8, 9]],
                                        [2, 3],
                                        [20, 30]))
            assert_equal(err.getvalue().strip(),
                         'Warning: while appending section: 1 to parent: 0\n'
                         'The section first point should be parent section last point: \n'
                         '        : X Y Z Diameter\n'
                         'parent last point :[4.000000, 5.000000, 6.000000, 2.000000]\n'
                         'child first point :[400.000000, 5.000000, 6.000000, 2.000000]')


def test_mut_copy_ctor():
    simple = Morphology(os.path.join(_path, "simple.swc"))
    assert_equal([sec.id for sec in simple.iter()],
                 [0, 1, 2, 3, 4, 5])
    copy = Morphology(simple)
    copy.append_root_section(PointLevel([[1, 2, 3], [4, 5, 6]],
                                        [2, 2],
                                        [20, 20]),
                             SectionType.axon)

    # test that first object has not been mutated
    assert_equal([sec.id for sec in simple.iter()],
                 [0, 1, 2, 3, 4, 5])

    assert_equal([sec.id for sec in copy.iter()],
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
    assert_equal(len(sections), 3)

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

    assert_equal(len(immutable_morphology.section(0).children), 2)

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
    morpho = ImmutableMorphology(os.path.join(_path, "simple.swc"))
    assert_array_equal(morpho.points, morpho.as_mutable().as_immutable().points)


def test_mitochondria_read():
    '''Read a H5 file with a mitochondria'''
    morpho = Morphology(os.path.join(_path, "h5/v1/mitochondria.h5"))
    mito = morpho.mitochondria
    assert_equal(len(mito.root_sections), 2)

    mitochondria = mito.root_sections

    assert_array_equal(mitochondria[0].diameters,
                       [10, 20])
    assert_array_equal(mitochondria[0].relative_path_lengths,
                       np.array([0.5, 0.6], dtype=np.float32))
    assert_array_equal(mitochondria[0].neurite_section_ids,
                       np.array([0., 0.], dtype=np.float32))

    assert_equal(len(mito.children(mito.root_sections[0])), 1)

    assert_equal(mito.parent(mito.children(mitochondria[0])[0]),
                 mitochondria[0])

    assert_array_equal(mitochondria[1].diameters,
                       [5, 6, 7, 8])
    assert_array_equal(mitochondria[1].relative_path_lengths,
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))
    assert_array_equal(mitochondria[1].neurite_section_ids,
                       np.array([0, 1, 1, 2], dtype=np.float32))

    assert_equal(len(mito.children(mito.root_sections[1])), 0)


def test_sections_are_not_dereferenced():
    '''There used to be a bug where if you would call:
    mitochondria.sections, that would dereference all section pointers
    if mitochondria.sections was not kept in a variable'''
    morpho = Morphology(os.path.join(_path, "h5/v1/mitochondria.h5"))

    # This lines used to cause a bug
    morpho.mitochondria.sections  # pylint: disable=pointless-statement

    ok_(all(section is not None for section in morpho.mitochondria.sections.values()))


def test_append_mutable_section():
    morpho = Morphology()
    second_children_first_root = SIMPLE.root_sections[0].children[1]

    morpho.append_root_section(second_children_first_root)
    assert_equal(len(morpho.root_sections), 1)

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

    assert_equal(mito.is_root(first_mito_id), True)
    assert_equal(mito.children(first_mito_id), [first_child])
    assert_equal(mito.parent(first_child), first_mito_id)
    assert_equal(mito.root_sections, [first_mito_id, second_mito_id])

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

    neuron = Morphology(os.path.join(_path, "iterators.asc"))
    root = neuron.root_sections[0]
    assert_array_equal([section.id for section in root.iter(IterType.depth_first)],
                       [0, 1, 2, 3, 4, 5, 6])
    assert_array_equal([section.id for section in root.iter(IterType.breadth_first)],
                       [0, 1, 4, 2, 3, 5, 6])

    assert_array_equal([section.id for section in neuron.iter(IterType.breadth_first)],
                       [0, 7, 1, 4, 8, 9, 2, 3, 5, 6])

def test_non_C_nparray():
    m = Morphology(os.path.join(_path, "simple.swc"))
    section = m.root_sections[0]
    points = np.array([[1, 2, 3], [4, 5, 6]])
    section.points = points
    assert_array_equal(section.points, points)

    with assert_raises(MorphioError) as obj:
        section.points = points.T

    assert_substring("Wrong array shape. Expected: (X, 3), got: (3, 2)",
                     str(obj.exception))

    non_standard_stride = np.asfortranarray(points)
    section.points = non_standard_stride
    assert_array_equal(section.points, points)

def test_annotation():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            with tmp_asc_file('''((Dendrite)
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
                 ''') as tmp_file:
                cell = Morphology(tmp_file.name)

    for n in (cell, cell.as_immutable(), cell.as_immutable().as_mutable()):
        assert_equal(len(n.annotations), 1)
        annotation = n.annotations[0]
        assert_equal(annotation.type, morphio.AnnotationType.single_child)

def test_section___str__():
    assert_equal(str(SIMPLE.root_sections[0]),
                 'Section(id=0, points=[(0 0 0),..., (0 5 0)])')


def test_from_pathlib():
    neuron = Morphology(Path(_path, "simple.asc"))
    assert_equal(len(neuron.root_sections), 2)


def test_endoplasmic_reticulum():
    neuron = Morphology(Path(_path, "simple.asc"))
    reticulum = neuron.endoplasmic_reticulum
    assert_equal(reticulum.section_indices, [])
    assert_equal(reticulum.volumes, [])
    assert_equal(reticulum.surface_areas, [])
    assert_equal(reticulum.filament_counts, [])

    reticulum.section_indices = [1, 1]
    reticulum.volumes = [2, 2]
    reticulum.surface_areas = [3, 3]
    reticulum.filament_counts = [4, 4]

    assert_equal(reticulum.section_indices, [1, 1])
    assert_equal(reticulum.volumes, [2, 2])
    assert_equal(reticulum.surface_areas, [3, 3])
    assert_equal(reticulum.filament_counts, [4, 4])

    with setup_tempdir('test-endoplasmic-reticulum') as folder:
        path = Path(folder, 'with-reticulum.h5')
        neuron.write(path)

        neuron = Morphology(path)
    reticulum = neuron.endoplasmic_reticulum
    assert_equal(reticulum.section_indices, [1, 1])
    assert_equal(reticulum.volumes, [2, 2])
    assert_equal(reticulum.surface_areas, [3, 3])
    assert_equal(reticulum.filament_counts, [4, 4])


def test_sanitize():
    m = Morphology()
    section = m.append_root_section(PointLevel([[1, 0, 0],
                                                [2, 0, 0]], [2, 2], [20, 20]),
                                    SectionType.axon)
    section.append_section(PointLevel([[2, 0, 0],
                                       [3, 0, 0]], [2, 2], [20, 20]))
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            m.sanitize()
            assert_equal(len(list(m.iter())), 1)
            assert_equal(err.getvalue().strip(),
                         'Warning: section 1 is the only child of section: 0\nIt will be merged '
                         'with the parent section')

    # Checking that sanitize() issues a warning on missing duplicate
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
            m.sanitize()
            assert_equal(err.getvalue().strip(),
                         'Warning: while appending section: 2 to parent: 0\nThe section first point should be parent section last point: \n        : X Y Z Diameter\nparent last point :[2.000000, 0.000000, 0.000000, 2.000000]\nchild first point :[2.000000, 1.000000, 0.000000, 2.000000]')


def test_glia():
    g = GlialCell()
    assert_equal(g.cell_family, CellFamily.GLIA)

    g = GlialCell(os.path.join(_path, 'astrocyte.h5'))
    assert_equal(g.cell_family, CellFamily.GLIA)

    g = GlialCell(Path(_path, 'astrocyte.h5'))
    assert_equal(g.cell_family, CellFamily.GLIA)

    assert_raises(RawDataError, GlialCell, Path(_path, 'simple.swc'))
    assert_raises(RawDataError, GlialCell, Path(_path, 'h5/v1/simple.h5'))


def test_glia_round_trip():
    with TemporaryDirectory() as folder:
        g = GlialCell(os.path.join(_path, 'astrocyte.h5'))
        filename = Path(folder, 'glial-cell.h5')
        g.write(filename)
        g2 = GlialCell(filename)
        assert_equal(len(g.sections), len(g2.sections))
