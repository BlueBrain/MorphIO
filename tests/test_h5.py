import os
import numpy as np
from numpy.testing import assert_array_equal
from nose import tools as nt

from morphio import Morphology, MORPHOLOGY_VERSION_H5_1, MORPHOLOGY_VERSION_H5_2

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
H5_PATH = os.path.join(_path, 'h5')
H5V1_PATH = os.path.join(H5_PATH, 'v1')
H5V2_PATH = os.path.join(H5_PATH, 'v2')


def test_v1():
    n = Morphology(os.path.join(H5V1_PATH, 'Neuron.h5'))
    nt.assert_equal(n.version, MORPHOLOGY_VERSION_H5_1)

    nt.assert_equal(len(list(n.iter())), 84)
    nt.assert_equal(len(n.points), 927)
    assert_array_equal(n.points[:10],
                       [[0.0, 0.0, 0.0],
                        [0.0, 0.20000000298023224, 0.0],
                        [0.10000000149011612, 0.10000000149011612, 0.0],
                        [0.0, 0.0, 0.0],
                        [0.0, 0.0, 0.10000000149011612],
                        [0.5529246926307678, -0.7534923553466797, 0.9035181403160095],
                        [1.2052767276763916, -1.3861794471740723, 1.6835479736328125],
                        [1.2670834064483643, -1.3914604187011719, 2.4186644554138184],
                        [1.271288275718689, -2.3130500316619873, 3.192789077758789],
                        [1.605881929397583, -2.6893420219421387, 3.992844343185425]])


def test_v2():
    n = Morphology(os.path.join(H5V2_PATH, 'Neuron.h5'))
    nt.assert_equal(n.version, MORPHOLOGY_VERSION_H5_2)

    nt.assert_equal(len(list(n.iter())), 85)
    nt.assert_equal(len(n.points), 927)
