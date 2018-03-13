import os

import numpy as np

from numpy.testing import assert_array_equal

from nose import tools as nt
from morphio import Morphology, SomaError, RawDataError


_path = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(_path, '../../../test_data')
SWC_PATH = os.path.join(DATA_PATH, 'swc')
SWC_SOMA_PATH = os.path.join(SWC_PATH, 'soma')

from utils import tmp_swc_file, _test_swc_exception

SWC_TEST_FILE = os.path.dirname(os.path.realpath(__file__))


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


def test_read_single_neurite():
    with tmp_swc_file('''# A simple neuron consisting of a point soma
                         # and a single branch neurite.
                         1 1 0 0 0 3.0 -1
                         2 3 0 0 2 0.5 1
                         3 3 0 0 3 0.5 2
                         4 3 0 0 4 0.5 3
                         5 3 0 0 5 0.5 4''') as tmp_file:

        n = Morphology(tmp_file.name)
    nt.eq_(len(n.rootSections), 1)
    nt.eq_(n.rootSections[0].id, 1)
    assert_array_equal(n.soma.points,
                       [[0, 0, 0]])
    nt.eq_(len(n.rootSections), 1)
    nt.eq_(len(n.sections), 2)
    assert_array_equal(n.rootSections[0].points,
                       np.array([[0, 0, 2],
                                 [0, 0, 3],
                                 [0, 0, 4],
                                 [0, 0, 5]]))


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
                         7 1 3 0 0 4.0 1
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

    nt.assert_equal(len(n.rootSections), 2)
    assert_array_equal(n.rootSections[0].points,
                       [[0, 0, 2],
                        [0, 0, 3],
                        [0, 0, 4],
                        [0, 0, 5]])

    assert_array_equal(n.rootSections[1].points,
                       [[0, 0, 6],
                        [0, 0, 7],
                        [0, 0, 8],
                        [0, 0, 9]])

    nt.eq_(len(n.sections), 3)


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

    simple = Morphology(os.path.join(SWC_TEST_FILE, 'simple.swc'))
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
    nt.assert_equal(len(n.rootSections), 2)
    assert_array_equal(n.rootSections[0].points,
                       [[0, 0, 0],
                        [0, 5, 0]])
    assert_array_equal(n.rootSections[1].points,
                       [[0, 0, 0],
                        [0, -4, 0]])
    assert_array_equal(n.rootSections[1].children[0].points,
                       [[0, -4, 0],
                        [6, -4, 0]])
    assert_array_equal(n.rootSections[1].children[1].points,
                       [[0, -4, 0],
                        [-5, -4, 0]])
