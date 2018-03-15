from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio.mut import Morphology


def test_writer():
    m = Morphology("simple.swc")
    m.write_swc("simple_written.swc")
