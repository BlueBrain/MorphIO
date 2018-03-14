from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio.mut import Morphology, Soma
from morphio import ostream_redirect, PointLevel, SectionType, SectionBuilderError, Morphology as ImmutableMorphology
from contextlib import contextmanager
import sys
from io import StringIO
from utils import assert_substring


def test_writer():
    m = Morphology()
    m.soma.points = [[-1, -2, -3]]
    m.soma.diameters = [-4]

    section_id = m.appendSection(-1,
                                 SectionType.axon,
                                 PointLevel([[1, 2, 3], [4, 5, 6]],
                                            [2, 2],
                                            [20, 20]))

    m.appendSection(section_id,
                    SectionType.axon,
                    PointLevel([[4, 5, 6], [7, 8, 9]],
                               [2, 3],
                               [20, 30]))
