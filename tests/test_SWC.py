import os
from nose.tools import assert_equal

from morphio import Morphology, ostream_redirect

from utils import captured_output, strip_color_codes


_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


def test_disconnected_neurite():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            n = Morphology(os.path.join(_path, 'disconnected_neurite.swc'))
            assert_equal(
                '''/home/bcoste/workspace/morphology/io/tests/data/disconnected_neurite.swc:10:warning
Found a disconnected neurite.
Neurites are not supposed to have parentId: -1
(although this is normal if this neuron has no soma)''',
                strip_color_codes(err.getvalue().strip()))
