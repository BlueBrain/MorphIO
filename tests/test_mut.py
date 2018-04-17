import numpy as np
from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio.mut import Morphology, Soma
from morphio import ostream_redirect, MitochondriaPointLevel, PointLevel, SectionType, SectionBuilderError, Morphology as ImmutableMorphology
from contextlib import contextmanager
import sys
from io import StringIO
from utils import assert_substring


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


def test_empty_neurite():
    m = Morphology()
    m.append_section(-1, SectionType.axon, PointLevel())
    assert_equal(len(m.root_sections), 1)
    assert_equal(m.section(m.root_sections[0]).type,
                 SectionType.axon)


def test_single_neurite():
    m = Morphology()
    m.append_section(-1,
                     SectionType.axon,
                     PointLevel([[1, 2, 3]], [2], [20]))

    assert_equal(m.section(m.root_sections[0]).points,
                 [[1, 2, 3]])
    assert_equal(m.section(m.root_sections[0]).diameters,
                 [2])
    assert_equal(m.section(m.root_sections[0]).perimeters,
                 [20])

    m.section(m.root_sections[0]).points = [[10, 20, 30]]
    assert_equal(m.section(m.root_sections[0]).points,
                 [[10, 20, 30]],
                 'Points array should have been mutated')

    m.section(m.root_sections[0]).diameters = [7]

    assert_equal(m.section(m.root_sections[0]).diameters,
                 [7],
                 'Diameter array should have been mutated')

    m.section(m.root_sections[0]).perimeters = [27]
    assert_equal(m.section(m.root_sections[0]).perimeters,
                 [27],
                 'Perimeter array should have been mutated')


def test_child_section():
    m = Morphology()
    section_id = m.append_section(-1,
                                  SectionType.axon,
                                  PointLevel([[1, 2, 3]], [2], [20]))

    m.append_section(section_id,
                     SectionType.axon,
                     PointLevel([[1, 2, 3], [4, 5, 6]],
                                [2, 3],
                                [20, 30]))

    children = m.children(m.root_sections[0])
    assert_equal(len(children),
                 1)

    assert_equal(m.section(children[0]).points,
                 [[1, 2, 3], [4, 5, 6]])
    assert_equal(m.section(children[0]).diameters,
                 [2, 3])
    assert_equal(m.section(children[0]).perimeters,
                 [20, 30])


@contextmanager
def captured_output():
    new_out, new_err = StringIO(), StringIO()
    old_out, old_err = sys.stdout, sys.stderr
    try:
        sys.stdout, sys.stderr = new_out, new_err
        yield sys.stdout, sys.stderr
    finally:
        sys.stdout, sys.stderr = old_out, old_err


def test_append_no_duplicate():
    m = Morphology()
    section_id = m.append_section(-1,
                                  SectionType.axon,
                                  PointLevel([[1, 2, 3], [4, 5, 6]],
                                             [2, 2],
                                             [20, 20]))

    with captured_output() as (out, err):
        with ostream_redirect():
            m.append_section(section_id,
                             SectionType.axon,
                             PointLevel([[400, 5, 6], [7, 8, 9]],
                                        [2, 3],
                                        [20, 30]))

    exception_str = ("While appending section: 1 to parent: 0\n"
                     "The section first point should be parent section last point:")

    # Testing the warning
    assert_substring(exception_str, err.getvalue())

    # Testing the exception
    with assert_raises(SectionBuilderError) as obj:
        m.build_read_only()
    assert_substring(exception_str, str(obj.exception))


def test_build_read_only():
    m = Morphology()
    m.soma.points = [[-1, -2, -3]]
    m.soma.diameters = [-4]

    section_id = m.append_section(-1,
                                  SectionType.axon,
                                  PointLevel([[1, 2, 3], [4, 5, 6]],
                                             [2, 2],
                                             [20, 20]))

    m.append_section(section_id,
                     SectionType.axon,
                     PointLevel([[4, 5, 6], [7, 8, 9]],
                                [2, 3],
                                [20, 30]))

    m.append_section(section_id,
                     SectionType.axon,
                     PointLevel([[4, 5, 6], [10, 11, 12]],
                                [2, 2],
                                [20, 20]))

    immutable_morphology = ImmutableMorphology(m)
    assert_equal(len(immutable_morphology.sections), 4)

    assert_array_equal(immutable_morphology.sections[0].points,
                       [[-1, - 2, -3]])
    assert_array_equal(immutable_morphology.sections[0].diameters,
                       [-4])

    # 0 is a fill value for soma "perimeter" property (which is undefined)
    assert_array_equal(immutable_morphology.sections[0].perimeters,
                       [0])

    assert_array_equal(immutable_morphology.sections[1].points,
                       [[1, 2, 3], [4, 5, 6]])
    assert_array_equal(immutable_morphology.sections[1].diameters,
                       [2, 2])
    assert_array_equal(immutable_morphology.sections[1].perimeters,
                       [20, 20])

    assert_equal(len(immutable_morphology.sections[1].children),
                 2)

    child = immutable_morphology.sections[1].children[0]
    assert_array_equal(child.points,
                       [[4, 5, 6], [7, 8, 9]])
    assert_array_equal(child.diameters,
                       [2, 3])
    assert_array_equal(child.perimeters,
                       [20, 30])

    same_child = immutable_morphology.sections[2]
    assert_array_equal(same_child.points,
                       [[4, 5, 6], [7, 8, 9]])
    assert_array_equal(same_child.diameters,
                       [2, 3])
    assert_array_equal(same_child.perimeters,
                       [20, 30])


def test_mitochondria():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [1, 1, 1]]
    morpho.soma.diameters = [1, 1]

    sectionId = morpho.append_section(
        -1, SectionType.axon,
        PointLevel([[2, 2, 2], [3, 3, 3]], [4, 4], [5, 5]))

    mito = morpho.mitochondria
    first_mito_id = mito.append_section(
        -1, MitochondriaPointLevel([0, 0], [0.5, 0.6],
                                   [10, 20]))

    first_child = mito.append_section(first_mito_id,
                                      MitochondriaPointLevel([3, 4, 4, 5],
                                                             [0.6, 0.7, 0.8, 0.9],
                                                             [20, 30, 40, 50]))

    second_mito_id = mito.append_section(-1,
                                         MitochondriaPointLevel([0, 1, 1, 2],
                                                                [0.6, 0.7, 0.8, 0.9],
                                                                [5, 6, 7, 8]))

    assert_equal(mito.parent(first_mito_id), -1)
    assert_equal(mito.children(first_mito_id), [first_child])
    assert_equal(mito.parent(first_child), first_mito_id)
    assert_equal(mito.root_sections, [first_mito_id, second_mito_id])

    assert_array_equal(mito.section(first_child).diameters,
                       [20, 30, 40, 50])
    assert_array_equal(mito.section(first_child).neurite_section_ids,
                       [3, 4, 4, 5])

    assert_array_equal(np.array(mito.section(first_child).path_lengths, dtype=np.float32),
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))
