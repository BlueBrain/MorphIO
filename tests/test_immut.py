import os
import numpy as np
from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio import Morphology

_path = os.path.dirname(os.path.abspath(__file__))


def test_equality():
    swc = Morphology(os.path.join(_path, "simple.swc"))
    asc = Morphology(os.path.join(_path, "simple.asc"))
    ok_(swc == asc)


def test_mitochondria():
    morpho = Morphology(os.path.join(_path, "h5/v1/mitochondria.h5"))
    assert_equal(len(morpho.mitochondria.root_sections), 2)

    mitochondria = [morpho.mitochondria.section(0),
                    morpho.mitochondria.section(1)]

    assert_array_equal(mitochondria[0].diameters,
                       [10, 20])
    assert_array_equal(mitochondria[0].relative_path_lengths,
                       np.array([0.5, 0.6], dtype=np.float32))
    assert_array_equal(mitochondria[0].neurite_section_id,
                       np.array([0., 0.], dtype=np.float32))

    assert_equal(len(mitochondria[0].children), 1)
    assert_equal(mitochondria[0].children[0].parent.id,
                 mitochondria[0].id)

    assert_array_equal(mitochondria[1].diameters,
                       [20, 30, 40, 50])
    assert_array_equal(mitochondria[1].relative_path_lengths,
                       np.array([0.6, 0.7, 0.8, 0.9], dtype=np.float32))
    assert_array_equal(mitochondria[1].neurite_section_id,
                       np.array([3., 4., 4., 5.], dtype=np.float32))

    assert_equal(len(mitochondria[1].children), 0)
