from nose.tools import assert_equal, assert_raises

from morphio.mut import Morphology
from morphio import PointLevel, SectionType, SectionBuilderError

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
    m.appendSection(-1, SectionType.axon, PointLevel())
    assert_equal(len(m.root_sections), 1)
    assert_equal(m.section(m.root_sections[0]).type,
                 SectionType.axon)


def test_single_neurite():
    m = Morphology()
    m.appendSection(-1,
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
    section_id = m.appendSection(-1,
                                 SectionType.axon,
                                 PointLevel([[1, 2, 3]], [2], [20]))

    m.appendSection(section_id,
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
