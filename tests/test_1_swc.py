# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path

import numpy as np
import morphio
import pytest
from numpy.testing import assert_array_equal
from utils import (assert_swc_exception, captured_output, ignored_warning,
                   strip_color_codes)

from morphio import (MorphioError, Morphology, RawDataError, SomaError,
                     SomaType, Warning, ostream_redirect, set_raise_warnings, Option)


DATA_DIR = Path(__file__).parent / "data"

def test_basic():
    contents =('''\
1 1 0 4 0 3.0 -1
2 3 0 0 2 0.5 1
''')
    Morphology(contents, "swc")


def test_build_from_string():
    contents =('''1 1 0 4 0 3.0 -1
                  2 3 0 0 2 0.5 1
                  3 3 0 0 3 0.5 2
                  4 3 0 0 4 0.5 3
                  5 3 0 0 5 0.5 4''')
    n = Morphology(contents, "swc")
    assert_array_equal(n.soma.points, [[0, 4, 0]])
    assert_array_equal(n.soma.diameters, [6.0])
    assert len(n.root_sections) == 1
    assert n.root_sections[0].id == 0
    assert_array_equal(n.root_sections[0].points,
                       np.array([[0, 0, 2],
                                 [0, 0, 3],
                                 [0, 0, 4],
                                 [0, 0, 5]]))
    assert_array_equal(n.section_offsets, [0, 4])


def test_read_single_neurite():
    '''Test reading a simple neuron consisting of a point soma and a single branch neurite.'''
    content = ('''1 1 0 4 0 3.0 -1
                  2 3 0 0 2 0.5 1
                  3 3 0 0 3 0.5 2
                  4 3 0 0 4 0.5 3
                  5 3 0 0 5 0.5 4''')
    n = Morphology(content, extension='swc')

    assert_array_equal(n.soma.points, [[0, 4, 0]])
    assert_array_equal(n.soma.diameters, [6.0])
    assert len(n.root_sections) == 1
    assert n.root_sections[0].id == 0
    assert_array_equal(n.root_sections[0].points,
                       np.array([[0, 0, 2],
                                 [0, 0, 3],
                                 [0, 0, 4],
                                 [0, 0, 5]]))
    assert_array_equal(n.section_offsets, [0, 4])


def test_skip_comments():
    '''Test skipping comments'''
    content = ('#a comment\n'
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
               '5 3 0 0 5 0.5 4\n')
    n = Morphology(content, extension='swc')
    assert_array_equal(n.soma.points, [[0, 4, 0]])
    assert_array_equal(n.soma.diameters, [6.0])

    assert len(n.root_sections) == 1
    assert_array_equal(n.root_sections[0].points,
                       np.array([[0, 0, 2],
                                 [0, 0, 3],
                                 [0, 0, 4],
                                 [0, 0, 5]]))

def test_read_simple():
    simple = Morphology(DATA_DIR /  'simple.swc')
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
            Morphology(DATA_DIR /  'disconnected_neurite.swc')
    finally:
        set_raise_warnings(False)


def test_repeated_id():
    assert_swc_exception( '''# A simple neuron with a repeated id
                         1 1 0 0 1 0.5 -1
                         2 3 0 0 2 0.5 1
                         3 3 0 0 3 0.5 2
                         4 3 0 0 4 0.5 3
                         4 3 0 0 4 0.5 3 # <-- repeated id
                         5 3 0 0 5 0.5 4
                         ''',
                         RawDataError,
                         'Repeated ID: 4\nID already appears here:',
                         ':4:warning')


def test_neurite_followed_by_soma():
    assert_swc_exception('''# An orphan neurite with a soma child
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
    content = ('''# A simple neuron consisting of a two-branch soma
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
                  ''')
    n = Morphology(content, extension='swc')

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
    content = (
                      """

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
""")
    n = Morphology(content, extension='swc')

    simple = Morphology(DATA_DIR /  'simple.swc')
    assert_array_equal(simple.points, n.points)


def test_cyclic():
    assert_swc_exception("""1 1  0  0 0 1. -1
                            2 3  0  0 0 1.  1
                            3 3  0  5 0 1.  2
                            4 3 -5  5 0 0.  3
                            5 3  6  5 0 0.  3
                            6 2  0  0 0 1.  6  # <-- cyclic point
                            7 2  0 -4 0 1.  6
                            8 2  6 -4 0 0.  7
                            9 2 -5 -4 0 0.  7
                         """,
                         RawDataError,
                         'Parent ID can not be itself',
                         ':6:error')


def test_simple_reversed():
    content = ('''# This is the same as 'simple.swc',
                  # except w/ leaf nodes before their parents
                  1 1  0  0 0 1. -1
                  2 3 -5  5 0 0.  7
                  3 3  6  5 0 0.  7
                  4 2  6 -4 0 0.  9
                  5 2 -5 -4 0 0.  9
                  6 3  0  0 0 1.  1
                  7 3  0  5 0 1.  6
                  8 2  0  0 0 1.  1
                  9 2  0 -4 0 1.  8 ''')
    n = Morphology(content, extension='swc')
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


def test_soma_type_1_point():
    content = '''1 1 0 0 0 3.0 -1'''
    assert (Morphology(content, extension='swc').soma_type ==
            SomaType.SOMA_SINGLE_POINT)


def test_soma_type_2_point():
    content = ('''1 1 0 0 0 3.0 -1
                  2 1 0 0 0 3.0  1''')
    assert (Morphology(content, extension='swc').soma_type ==
            SomaType.SOMA_CYLINDERS)


def test_soma_type_many_point():
    content = ('''1 1 0 0 0 3.0 -1
                  2 1 0 0 0 3.0  1
                  3 1 0 0 0 3.0  2
                  4 1 0 0 0 3.0  3
                  5 1 0 0 0 3.0  4''')
    assert (Morphology(content, extension='swc').soma_type ==
            SomaType.SOMA_CYLINDERS)

def test_soma_type_3_point_neuromorpho(tmp_path):
    # 3 points soma can be of type SOMA_CYLINDERS or SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS
    # depending on the point layout

    # SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS are characterized by
    # one soma point with 2 children
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            content = ('''1 1 0  0 0 3.0 -1
                          2 1 0 -3 0 3.0  1
                          3 1 0  3 0 3.0  1 # PID is 1''')
            m = Morphology(content, extension='swc')
            assert (m.soma_type ==
                    SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
            assert len(err.getvalue()) == 0

            m = m.as_mutable()
            m.write(tmp_path / 'test_soma_type_3_point.swc')
            assert len(err.getvalue()) == 0

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            content = ('''1 1 0  0 0 3.0 -1
                          2 1 1 -3 0 3.0  1 # <- note y is 1 instead of 0
                          3 1 0  0 0 3.0  1 # PID is 1''')
            assert (Morphology(content, extension='swc').soma_type ==
                    SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
            assert strip_color_codes(err.getvalue()).strip() == (
                    "$STRING$:0:warning\n"
                    "Three Point Soma: Only one column has the same coordinates."
                    )

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            content = ('''1 1 0  0 0 3.0 -1
                          2 1 0 -3 0 3.0  1 # <- note z is -3 instead of 0
                          3 1 0  0 0 3.0  1 # PID is 1''')
            assert (Morphology(content, extension='swc').soma_type ==
                         SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
            assert strip_color_codes(err.getvalue()).strip() == (
                    "$STRING$:0:warning\n"
                    "Three Point Soma: The non-constant columns is not offset by +/- the radius from the initial sample."
                    )

def test_soma_type_3_point():
    # If this configuration is not respected -> SOMA_CYLINDERS
    content = ( '''1 1 0 0 0 3.0 -1
                   2 1 0 0 0 3.0  1
                   3 1 0 0 0 3.0  2 # PID is 2''')
    assert (Morphology(content, extension='swc').soma_type ==
            SomaType.SOMA_CYLINDERS)


def test_swc_threepoint_soma_tolerance():
    # from https://github.com/BlueBrain/MorphIO/issues/492
    contents = """
    1 1 5789.674999999998 1322.85 2846.65 5.165118027 -1
    2 1 5789.674999999998 1317.6848819729996 2846.65 5.165118027 1
    3 1 5789.674999999998 1328.015118027 2846.65 5.165118027 1
    """
    warnings = morphio.WarningHandlerCollector()
    Morphology(contents, extension="swc", warning_handler=warnings)
    assert len(warnings.get_all()) == 0

    contents = """
    1 1 10000 10000             10000 100.999999999999999999 -1
    2 1 10000 9899.000000000001 10000 100.999999999999999999 1
    3 1 10000 10100.99999999999 10000 100.999999999999999999 1
    """
    warnings = morphio.WarningHandlerCollector()
    Morphology(contents, extension="swc", warning_handler=warnings)
    assert len(warnings.get_all()) == 0

    # This still passes, even with the third point being off by 0.0000008 which seems reasonable
    contents = """
    1 1 10000 10000             10000 100.999999999999999999 -1
    2 1 10000 9899.000000000001 10000 100.999999999999999999 1
    3 1 10000 10100.99999911111 10000 100.999999999999999999 1
    """
    warnings = morphio.WarningHandlerCollector()
    Morphology(contents, extension="swc", warning_handler=warnings)
    assert len(warnings.get_all()) == 0

    # The third point being off by 0.0088888 creates warning
    contents = """
    1 1 10000 10000             10000 100.999999999999999999 -1
    2 1 10000 9899.000000000001 10000 100.999999999999999999 1
    3 1 10000 10100.99111111111 10000 100.999999999999999999 1
    """
    warnings = morphio.WarningHandlerCollector()
    Morphology(contents, extension="swc", warning_handler=warnings)
    assert len(warnings.get_all()) == 1


def test_read_weird_ids():
    '''The ordering of IDs is not required'''
    content = ('''10000 3 0 0 5 0.5 100 # neurite 4th point
                  3 3 0 0 3 0.5 47      # neurite 2nd point
                  10 1 0 0 0 3.0 -1     # soma
                  47 3 0 0 2 0.5 10     # neurite 1st point
                  100 3 0 0 4 0.5 3     # neurite 3rd point
                  ''')
    neuron = Morphology(content, extension='swc')

    assert_array_equal(neuron.soma.points, [[0, 0, 0]])
    assert_array_equal(neuron.root_sections[0].points, [[0., 0., 2.],
                                                        [0., 0., 3.],
                                                        [0., 0., 4.],
                                                        [0., 0., 5.]])


def test_multiple_soma():
    with pytest.raises(SomaError, match='Multiple somata found') as e:
        Morphology(DATA_DIR /  'multiple_soma.swc')
    assert e.match('multiple_soma.swc:2:error')
    assert e.match('multiple_soma.swc:11:error')


def test_disconnected_neurite():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            path = DATA_DIR /  'disconnected_neurite.swc'
            Morphology(path)
            assert (
                f'''{path}:10:warning
Warning: found a disconnected neurite.
Neurites are not supposed to have parentId: -1
(although this is normal if this neuron has no soma)''' ==
                strip_color_codes(err.getvalue().strip()))


def test_neurite_wrong_root_point_neuromorpho_3_point_soma():
    '''Test that for 3 points soma, the neurites are attached to first soma point'''
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            path = DATA_DIR /  'neurite_wrong_root_point.swc'
            n = Morphology(path)
    assert strip_color_codes(err.getvalue().strip()) == f'''\
Warning: with a 3 points soma, neurites must be connected to the first soma point:
{path}:2:warning

Warning: with a 3 points soma, neurites must be connected to the first soma point:
{path}:3:warning'''
    assert len(n.root_sections) == 2
    assert_array_equal(n.root_sections[0].points,
                       [[0,0,0], [0,0,1]])

    with ignored_warning(Warning.wrong_root_point):
        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                n = Morphology(DATA_DIR /  'neurite_wrong_root_point.swc')
                assert err.getvalue().strip() == ''


def test_read_duplicate():
    '''Test reading a simple neuron consisting of a point soma
    and a single branch neurite.'''
    content = ('''# A simple neuron with a duplicate point
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
                  9 3 1 1 0 3.5 8''')

    n = Morphology(content, extension='swc')

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
    content = ('''1 1 0 4 0 3.0 -1
                  2 3 0 0 2 0.5 1
                  3 -1 0 0 3 0.5 2  # <-- -1 is unsupported section type
                  ''')

    with pytest.raises(RawDataError, match=':3:error') as obj:
        Morphology(content, extension='swc')
    assert obj.match('Unsupported section type: -1')

    content = ('''1 1 0 4 0 3.0 -1
                  2 3 0 0 2 0.5 1
                  3 20 0 0 3 0.5 2  # <-- 20 is unsupported section type
                  ''')

    with pytest.raises(RawDataError, match=':3:error') as obj:
        Morphology(content, extension='swc')
    assert obj.match('Unsupported section type: 20')


def test_three_point_soma_neuromorpho():
    n = Morphology(DATA_DIR /  'three_point_soma.swc')
    assert n.soma_type == SomaType.SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS


def test_three_point_soma_stacked_cylinders():
    n = Morphology(DATA_DIR /  'soma_cylinders.swc')
    assert n.soma_type == SomaType.SOMA_CYLINDERS
    assert len(n.root_sections) == 1


def test_trailing_space():
    n = Morphology(DATA_DIR / 'simple-trailing-space.swc')
    assert n.points.shape == (3, 3)


def test_zero_diameter():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            content = '''1 1 1 0 0 3.0 -1
                          2 1 2 0 0 3.0  1
                          3 1 3 0 0 3.0  2
                          4 3 4 0 0 0.0  1
                          5 3 5 0 0 3.0  4
                       '''
            Morphology(content, extension='swc')
            assert strip_color_codes(err.getvalue().strip()) == \
                '$STRING$:4:warning\nWarning: zero diameter in file'


def test_version():
    assert_array_equal(Morphology(DATA_DIR /  'simple.swc').version,
                       ('swc', 1, 0))


def test_no_soma():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            Morphology("", extension='swc')
    assert ('$STRING$:0:warning\nWarning: no soma found in file' ==
            strip_color_codes(err.getvalue().strip()))

    content = '''1 2 0 0 0 3.0 -1
                 2 2 0 0 0 3.0  1
                 3 2 0 0 0 3.0  2'''
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            Morphology(content, extension='swc')
    assert ('''$STRING$:1:warning
Warning: found a disconnected neurite.
Neurites are not supposed to have parentId: -1
(although this is normal if this neuron has no soma)

$STRING$:0:warning
Warning: no soma found in file''' ==

            strip_color_codes(err.getvalue().strip()))

def test_throw_on_missing_data():
    content = ''' 3    #    missing data '''
    with pytest.raises(RawDataError, match='Unable to parse this line'):
        Morphology(content, extension='swc')

    content = '''
    # some pre data
    3 3   #    missing data '''
    with pytest.raises(RawDataError, match='Unable to parse this line'):
        Morphology(content, extension='swc')


def test_throw_on_negative_id():
    content = '''1 2 0 0 0 3.0 -1
                 -2 2 0 0 0 3.0  1
                 3 2 0 0 0 3.0  2'''
    with pytest.raises(RawDataError, match='The ID assigned to this line is negative'):
        Morphology(content, extension='swc')


def test_axon_carrying_dendrite():
    contents =('''
    1 1 0 0 1 1 -1
    2 2 0 0 2 2 1
    3 2 0 0 3 3 2

    4 3 0 0 4 4 3  # dendrite splits off
    5 3 0 0 5 5 4

    6 2 0 0 6 6 3  # axon carries on
    7 2 0 0 7 7 3
               ''')
    Morphology(contents, "swc")


def test_multi_type_section():
    """
    A section within MorphIO is defined as a series of segments between
    bifurcation/multifurcation points.  However, SWC files allow section
    change without a branch. Normal parsing of this will raise an
    exception, but can be allowed using the option `UNIFURCATED_SECTION_CHANGE`
    """
    contents =('''1 1 0 4 0 3.0 -1
                  2 6 0 0 2 0.5 1        # <- type 6
                  3 7 0 0 3 0.5 2        # <- type 7
                  4 8 0 0 4 0.5 3        # <- type 8
                  5 9 0 0 5 0.5 4''')    # <- type 9

    with pytest.raises(RawDataError):
        Morphology(contents, "swc")

    warnings = morphio.WarningHandlerCollector()
    n = Morphology(contents,
                   "swc",
                   warning_handler=warnings,
                   options=Option.allow_unifurcated_section_change)
    assert_array_equal(n.soma.points, [[0, 4, 0]])
    assert_array_equal(n.soma.diameters, [6.0])
    assert len(n.root_sections) == 1
    assert n.root_sections[0].id == 0
    assert_array_equal(n.root_sections[0].points,
                       np.array([[0, 0, 2], ]))
    assert len(n.sections) == 4
    assert_array_equal(n.section_offsets, [0, 1, 3, 5, 7])
    warnings = [f.warning for f in warnings.get_all()]
    assert len(warnings) == 3  # type 7, 8, and 9
    for warning in warnings:
        assert warning.warning() == Warning.type_changed_within_section


def test_missing_parent():
    contents =('''
1 1  0  0 0 10 -1
2 2 -2 -6 0 10  1
3 2  2  6 0 10  2
4 2  2  6 0 10  10
''')
    with pytest.raises(morphio.MissingParentError, match='Sample id: 4 refers to non-existant parent ID: 10'):
        Morphology(contents, "swc")


def test_extra_column():
    # some SWC files include extra columns; this goes outside the spec, but we will allow it for
    # backwards compatibility
    contents =('''
1 1  0  0 0 10 -1  3
2 2 -2 -6 0 10  1  3
''')
    Morphology(contents, "swc")

def test_read_simple_windows_eol():
    simple = Morphology(DATA_DIR /  'simple-windows-eol.swc')
    assert len(simple.root_sections) == 2
    assert simple.root_sections[0].id == 0
    assert simple.root_sections[1].id == 3


def test_WarningHandlerCollector():
    warnings = morphio.WarningHandlerCollector()
    Morphology(DATA_DIR /  'neurite_wrong_root_point.swc', warning_handler=warnings)
    warnings = warnings.get_all()
    assert len(warnings) == 2
    assert [False, False] == [e.was_marked_ignore for e in warnings]
    assert warnings[0].warning.line_numbers[0] == 2
    assert warnings[1].warning.line_numbers[0] == 3

    warnings0 = morphio.WarningHandlerCollector()
    warnings1 = morphio.WarningHandlerCollector()
    Morphology(DATA_DIR /  'neurite_wrong_root_point.swc', warning_handler=warnings0)
    Morphology("", extension="swc", warning_handler=warnings1)
    assert len(warnings0.get_all()) == 2
    assert len(warnings1.get_all()) == 1
