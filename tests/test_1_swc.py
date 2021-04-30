import os

import numpy as np
import pytest
from numpy.testing import assert_array_equal

from morphio import (Morphology, RawDataError, SectionType, SomaError, MorphioError, SomaType,
                     ostream_redirect, set_maximum_warnings, set_raise_warnings, set_ignored_warning, Warning)
from utils import (_test_swc_exception, assert_string_equal, captured_output,
                   strip_color_codes, tmp_swc_file, ignored_warning)

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


def test_read_single_neurite():
    '''Test reading a simple neuron consisting of a point soma
    and a single branch neurite.'''
    with tmp_swc_file('''1 1 0 4 0 3.0 -1
                         2 3 0 0 2 0.5 1
                         3 3 0 0 3 0.5 2
                         4 3 0 0 4 0.5 3
                         5 3 0 0 5 0.5 4''') as tmp_file:
        n = Morphology(tmp_file.name)

    assert_array_equal(n.soma.points, [[0, 4, 0]])
    assert_array_equal(n.soma.diameters, [6.0])
    assert len(n.root_sections) == 1
    assert n.root_sections[0].id == 0
    assert len(n.root_sections) == 1
    assert_array_equal(n.root_sections[0].points,
                       np.array([[0, 0, 2],
                                 [0, 0, 3],
                                 [0, 0, 4],
                                 [0, 0, 5]]))
    assert_array_equal(n.section_offsets, [0, 4])


def test_skip_comments():
    '''Test skipping comments'''
    with tmp_swc_file(('#a comment\n'
                       '# a comment\n'
                       ' # a comment\n'
                       '  #  a comment\n'
                       '1 1 0 4 0 3.0 -1\n'
                       '\t#a tab comment\n'
                       '\t#\ta tab comment\n'
                       '\t\t#\t\ta a tab comment\n'
                       '2 3 0 0 2 0.5 1\n'
                       '3 3 0 0 3 0.5 2\n'
                       '4 3 0 0 4 0.5 3\n'
                       '5 3 0 0 5 0.5 4\n')) as tmp_file:
        n = Morphology(tmp_file.name)
    assert_array_equal(n.soma.points, [[0, 4, 0]])
    assert_array_equal(n.soma.diameters, [6.0])

    assert len(n.root_sections) == 1
    assert_array_equal(n.root_sections[0].points,
                       np.array([[0, 0, 2],
                                 [0, 0, 3],
                                 [0, 0, 4],
                                 [0, 0, 5]]))

def test_read_simple():
    simple = Morphology(os.path.join(_path, 'simple.swc'))
    assert len(simple.root_sections) == 2
    assert simple.root_sections[0].id == 0
    assert simple.root_sections[1].id == 3

    assert_array_equal(simple.root_sections[0].points, [[0, 0, 0], [0, 5, 0]])

    assert len(simple.root_sections[0].children) == 2
    assert simple.root_sections[0].children[0].id == 1
    assert simple.root_sections[0].children[1].id == 2
    assert_array_equal(simple.root_sections[0].children[0].points, [[0, 5, 0], [-5, 5, 0]])
    assert_array_equal(simple.root_sections[1].points, [[0, 0, 0], [0, -4, 0]])


def test_set_raise_warnings():
    try:
        set_raise_warnings(True)
        with pytest.raises(MorphioError, match='Warning: found a disconnected neurite'):
            Morphology(os.path.join(_path, 'disconnected_neurite.swc'))
    finally:
        set_raise_warnings(False)


def test_repeated_id():
    _test_swc_exception('''# A simple neuron with a repeated id
                       1 1 0 0 1 0.5 -1
                       2 3 0 0 2 0.5 1
                       3 3 0 0 3 0.5 2
                       4 3 0 0 4 0.5 3
                       4 3 0 0 4 0.5 3 # <-- repeated id
                       5 3 0 0 5 0.5 4
                       ''',
                        RawDataError,
                        'Repeated ID: 4\nID already appears here:',
                        ':6:warning')


def test_neurite_followed_by_soma():
    # Capturing the output to keep the unit test suite stdout clean
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            _test_swc_exception('''# An orphan neurite with a soma child
                           1 3 0 0 1 0.5 -1
                           2 3 0 0 2 0.5 1
                           3 3 0 0 3 0.5 2
                           4 3 0 0 4 0.5 3
                           5 3 0 0 5 0.5 4
                           6 1 0 0 0 3.0 5 # <-- soma child''',
                                SomaError,
                                'Found a soma point with a neurite as parent',
                                ':7:error')


def test_read_split_soma():
    with tmp_swc_file('''# A simple neuron consisting of a two-branch soma
                         # with a single branch neurite on each branch.
                         #
                         # initial soma point
                         1 1 1 0 1 4.0 -1
                         # first neurite
                         2 3 0 0 2 0.5 1
                         3 3 0 0 3 0.5 2
                         4 3 0 0 4 0.5 3
                         5 3 0 0 5 0.5 4
                         # soma branch, off initial point
                         6 1 2 0 0 4.0 1
                         7 1 3 0 0 4.0 6
                         # second neurite, off soma branch
                         8 3 0 0 6 0.5 1
                         9 3 0 0 7 0.5 8
                         10 3 0 0 8 0.5 9
                         11 3 0 0 9 0.5 10
                         ''') as tmp_file:
        n = Morphology(tmp_file.name)

    assert_array_equal(n.soma.points,
                       [[1, 0, 1],
                        [2, 0, 0],
                        [3, 0, 0]])

    assert len(n.root_sections) == 2
    assert_array_equal(n.root_sections[0].points,
                       [[0, 0, 2],
                        [0, 0, 3],
                        [0, 0, 4],
                        [0, 0, 5]])

    assert_array_equal(n.root_sections[1].points,
                       [[0, 0, 6],
                        [0, 0, 7],
                        [0, 0, 8],
                        [0, 0, 9]])

    assert len(list(n.iter())) == 2


def test_weird_indent():

    with tmp_swc_file("""

                 # this is the same as simple.swc

# but with a questionable styling

     1 1  0  0 0 1. -1
 2 3  0  0 0 1.  1

 3 3  0  5 0 1.  2
 4 3 -5  5 0 0.  3



 5 3  6  5 0 0.  3
     6 2  0  0 0 1.  1
 7 2  0 -4 0 1.  6

 8 2  6 -4 0         0.  7
 9 2 -5      -4 0 0.  7 # 3 0 0
""") as tmp_file:
        n = Morphology(tmp_file.name)

    simple = Morphology(os.path.join(_path, 'simple.swc'))
    assert_array_equal(simple.points,
                       n.points)


def test_cyclic():
    _test_swc_exception("""1 1  0  0 0 1. -1
                           2 3  0  0 0 1.  1
                           3 3  0  5 0 1.  2
                           4 3 -5  5 0 0.  3
                           5 3  6  5 0 0.  3
                           6 2  0  0 0 1.  6  # <-- cyclic point
                           7 2  0 -4 0 1.  6
                           8 2  6 -4 0 0.  7
                           9 2 -5 -4 0 0.  7""",
                        RawDataError,
                        'Parent ID can not be itself',
                        ':6:error')


def test_simple_reversed():
    with tmp_swc_file('''# This is the same as 'simple.swc',
                         # except w/ leaf nodes before their parents
                         1 1  0  0 0 1. -1
                         2 3 -5  5 0 0.  7
                         3 3  6  5 0 0.  7
                         4 2  6 -4 0 0.  9
                         5 2 -5 -4 0 0.  9
                         6 3  0  0 0 1.  1
                         7 3  0  5 0 1.  6
                         8 2  0  0 0 1.  1
                         9 2  0 -4 0 1.  8 ''') as tmp_file:
        n = Morphology(tmp_file.name)
    assert_array_equal(n.soma.points,
                       [[0, 0, 0]])
    assert len(n.root_sections) == 2
    assert_array_equal(n.root_sections[0].points,
                       [[0, 0, 0],
                        [0, 5, 0]])
    assert_array_equal(n.root_sections[1].points,
                       [[0, 0, 0],
                        [0, -4, 0]])
    assert_array_equal(n.root_sections[1].children[0].points,
                       [[0, -4, 0],
                        [6, -4, 0]])
    assert_array_equal(n.root_sections[1].children[1].points,
                       [[0, -4, 0],
                        [-5, -4, 0]])


def test_soma_type():
    '''The ordering of IDs is not required'''
    # 1 point soma
    with tmp_swc_file('''1 1 0 0 0 3.0 -1''') as tmp_file:
        assert (Morphology(tmp_file.name).soma_type ==
                     SomaType.SOMA_SINGLE_POINT)

    # 2 point soma
    with tmp_swc_file('''1 1 0 0 0 3.0 -1
                         2 1 0 0 0 3.0  1''') as tmp_file:
        assert (Morphology(tmp_file.name).soma_type ==
                     SomaType.SOMA_CYLINDERS)

    # > 3 points soma
    with tmp_swc_file('''1 1 0 0 0 3.0 -1
                         2 1 0 0 0 3.0  1
                         3 1 0 0 0 3.0  2
                         4 1 0 0 0 3.0  3
                         5 1 0 0 0 3.0  4''') as tmp_file:
        assert (Morphology(tmp_file.name).soma_type ==
                     SomaType.SOMA_CYLINDERS)

    # 3 points soma can be of type SOMA_CYLINDERS or SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS
    # depending on the point layout

    # SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS are characterized by
    # one soma point with 2 children
    with tmp_swc_file('''1 1 0  0 0 3.0 -1
    2 1 0 -3 0 3.0  1
    3 1 0  3 0 3.0  1 # PID is 1''') as tmp_file:
        assert (Morphology(tmp_file.name).soma_type ==
                     SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            with tmp_swc_file('''1 1 0  0 0 3.0 -1
                                 2 1 1 -3 0 3.0  1
                                 3 1 0  0 0 3.0  1 # PID is 1''') as tmp_file:
                assert (Morphology(tmp_file.name).soma_type ==
                             SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
                assert_string_equal('', err.getvalue())

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            with tmp_swc_file('''1 1 0  0 0 3.0 -1
                                 2 1 0 -3 0 3.0  1
                                 3 1 0  0 0 3.0  1 # PID is 1''') as tmp_file:
                assert (Morphology(tmp_file.name).soma_type ==
                             SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
                assert_string_equal(
                    '''{}:0:warning
                       Warning: the soma does not conform the three point soma spec
                       The only valid neuro-morpho soma is:
                       1 1 x   y   z r -1
                       2 1 x (y-r) z r  1
                       3 1 x (y+r) z r  1

                       Got:
                       1 1 0 0 0 3 -1
                       2 1 0.000000 -3.000000 0.000000 3.000000 1
                       3 1 0.000000 0.000000 (exp. 3.000000) 0.000000 3.000000 1'''.format(tmp_file.name),
                    err.getvalue(),
				)

# If this configuration is not respected -> SOMA_CYLINDERS
    with tmp_swc_file('''1 1 0 0 0 3.0 -1
                         2 1 0 0 0 3.0  1
                         3 1 0 0 0 3.0  2 # PID is 2''') as tmp_file:
        assert (Morphology(tmp_file.name).soma_type ==
                     SomaType.SOMA_CYLINDERS)


def test_read_weird_ids():
    '''The ordering of IDs is not required'''
    with tmp_swc_file('''10000 3 0 0 5 0.5 100 # neurite 4th point
                         3 3 0 0 3 0.5 47      # neurite 2nd point
                         10 1 0 0 0 3.0 -1     # soma
                         47 3 0 0 2 0.5 10     # neurite 1st point
                         100 3 0 0 4 0.5 3     # neurite 3rd point
                         ''') as tmp_file:
        neuron = Morphology(tmp_file.name)

    assert_array_equal(neuron.soma.points, [[0, 0, 0]])
    assert_array_equal(neuron.root_sections[0].points, [[0., 0., 2.],
                                                        [0., 0., 3.],
                                                        [0., 0., 4.],
                                                        [0., 0., 5.]])


def test_multiple_soma():
    with pytest.raises(SomaError, match='Multiple somata found') as e:
        Morphology(os.path.join(_path, 'multiple_soma.swc'))
    assert e.match('multiple_soma.swc:2:error')
    assert e.match('multiple_soma.swc:11:error')


def test_disconnected_neurite():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            n = Morphology(os.path.join(_path, 'disconnected_neurite.swc'))
            assert (
                '''{}:10:warning
Warning: found a disconnected neurite.
Neurites are not supposed to have parentId: -1
(although this is normal if this neuron has no soma)'''.format(os.path.join(_path, 'disconnected_neurite.swc')) ==
                strip_color_codes(err.getvalue().strip()))

def test_neurite_wrong_root_point():
    '''Test that for 3 points soma, the neurites are attached to first soma point'''

    # Not 3-points soma --> OK
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            n = Morphology(os.path.join(_path, 'soma_cylinders.swc'))
            assert err.getvalue().strip() == ''
        assert len(n.root_sections) == 1

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            path = os.path.join(_path, 'neurite_wrong_root_point.swc')
            n = Morphology(path)
            assert_string_equal(
'''Warning: with a 3 points soma, neurites must be connected to the first soma point:
{}:4:warning

{}:6:warning'''.format(path, path),
                err.getvalue())
    assert len(n.root_sections) == 2
    assert_array_equal(n.root_sections[0].points,
                       [[0,0,0], [0,0,1]])

    with ignored_warning(Warning.wrong_root_point):
        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                n = Morphology(os.path.join(_path, 'neurite_wrong_root_point.swc'))
                assert err.getvalue().strip() == ''


def test_read_duplicate():
    '''Test reading a simple neuron consisting of a point soma
    and a single branch neurite.'''
    with tmp_swc_file('''# A simple neuron with a duplicate point
                         # at the bifurcation
                         #
                         # soma point
                         1 1 1 0 1 4.0 -1
                         # root section
                         2 3 0 0 2 0.5 1
                         3 3 0 0 3 0.5 2
                         # first child: real duplicate
                         4 3 0 0 3 0.5 3
                         5 3 0 0 7 0.5 4
                         # second child: duplicate with different diameter
                         6 3 0 0 3 2.3 3
                         7 3 0 0 8 3.5 6
                         # third child: no duplicate
                         8 3 1 0 0 2.3 3
                         9 3 1 1 0 3.5 8''') as tmp_file:

        n = Morphology(tmp_file.name)

    assert len(n.root_sections) == 1
    assert len(n.root_sections[0].children) == 3
    child1, child2, child3 = n.root_sections[0].children
    assert_array_equal(n.root_sections[0].points, np.array([[0, 0, 2], [0, 0, 3]]))
    assert_array_equal(child1.points, np.array([[0, 0, 3], [0, 0, 7]]))
    assert_array_equal(child2.points, np.array([[0, 0, 3], [0, 0, 8]]))
    assert_array_equal(child3.points, np.array([[0, 0, 3],
                                                [1, 0, 0],
                                                [1, 1, 0]]))
    assert_array_equal(child1.diameters, np.array([1, 1]))
    assert_array_equal(child2.diameters, np.array([4.6, 7], dtype=np.float32))
    assert_array_equal(child3.diameters, np.array([1, 4.6, 7], dtype=np.float32))


def test_unsupported_section_type():
    with tmp_swc_file('''1 1 0 4 0 3.0 -1
                         2 3 0 0 2 0.5 1
                         3 -1 0 0 3 0.5 2  # <-- -1 is unsupported section type
                         ''') as tmp_file:

        with pytest.raises(RawDataError, match='.swc:3:error') as obj:
            Morphology(tmp_file.name)
        assert obj.match('Unsupported section type: -1')

    with tmp_swc_file('''1 1 0 4 0 3.0 -1
                         2 3 0 0 2 0.5 1
                         3 11 0 0 3 0.5 2  # <-- 11 is unsupported section type
                         ''') as tmp_file:

        with pytest.raises(RawDataError, match='.swc:3:error') as obj:
            Morphology(tmp_file.name)
        assert obj.match('Unsupported section type: 11')


def test_root_node_split():
    '''Test that a bifurcation at the root point produces
    two root sections
    '''
    with tmp_swc_file('''1	1	0 0 0 1	-1
                         2	3	1 0 0 1  1
                         3	3	1 1 0 1  2
                         4	3	1 0 1 1  2
                         ''') as tmp_file:
        n = Morphology(tmp_file.name)
        assert len(n.root_sections) == 2
        assert_array_equal(n.root_sections[0].points,
                           [[1, 0, 0], [1, 1, 0]])
        assert_array_equal(n.root_sections[1].points,
                           [[1, 0, 0], [1, 0, 1]])

    # Normal bifurcation
    with tmp_swc_file('''1	1	0 0 0 1	-1
                         2	3	1 0 0 1  1
                         3	3	2 1 0 1  2
                         4	3	1 1 0 1  3
                         5	3	1 0 1 1  3
                         ''') as tmp_file:
        n = Morphology(tmp_file.name)
        assert len(n.root_sections) == 1
        root = n.root_sections[0]
        assert_array_equal(root.points,
                           [[1, 0, 0], [2, 1, 0]])
        assert len(root.children) == 2
        assert_array_equal(root.children[0].points,
                           [[2, 1, 0], [1, 1, 0]])
        assert_array_equal(root.children[1].points,
                           [[2, 1, 0], [1, 0, 1]])


def test_three_point_soma():
    n = Morphology(os.path.join(_path, 'three_point_soma.swc'))
    assert n.soma_type == SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS


def test_zero_diameter():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True),\
                tmp_swc_file('''1 1 1 0 0 3.0 -1
                                2 1 2 0 0 3.0  1
                                3 1 3 0 0 3.0  2
                                4 3 4 0 0 0.0  1
                                5 3 5 0 0 3.0  4
                         ''') as tmp_file:
            Morphology(tmp_file.name)
            assert_string_equal(
                f'{tmp_file.name}:4:warning\nWarning: zero diameter in file\n',
                err.getvalue())



def test_version():
    assert_array_equal(Morphology(os.path.join(_path, 'simple.swc')).version,
                       ('swc', 1, 0))


def test_no_soma():
    swc_content = '''1 2 0 0 0 3.0 -1
                     2 2 0 0 0 3.0  1
                     3 2 0 0 0 3.0  2'''
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True), tmp_swc_file(swc_content) as tmp_file:
            n = Morphology(tmp_file.name)
            assert ('{}:0:warning\nWarning: no soma found in file'.format(tmp_file.name) ==
                    strip_color_codes(err.getvalue().strip()))
