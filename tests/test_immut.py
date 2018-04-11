import os
from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio import Morphology

_path = os.path.dirname(os.path.abspath(__file__))


def test_equality():
    swc = Morphology(os.path.join(_path, "simple.swc"))
    asc = Morphology(os.path.join(_path, "simple.asc"))
    ok_(swc == asc)
