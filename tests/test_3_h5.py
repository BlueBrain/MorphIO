import os
from itertools import chain, repeat

from nose.tools import assert_equal, assert_raises
from numpy.testing import assert_array_equal

from morphio import (Morphology, MorphologyVersion, RawDataError, SectionType,
                     ostream_redirect)
from . utils import captured_output

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
H5_PATH = os.path.join(_path, 'h5')
H5V1_PATH = os.path.join(H5_PATH, 'v1')
H5V2_PATH = os.path.join(H5_PATH, 'v2')


def test_v1():
    n = Morphology(os.path.join(H5V1_PATH, 'simple.h5'))
    assert_equal(len(n.root_sections), 2)
    assert_equal(n.root_sections[0].type, 3)
    assert_equal(n.root_sections[1].type, 2)

    n = Morphology(os.path.join(H5V1_PATH, 'Neuron.h5'))
    assert_equal(n.version, MorphologyVersion.MORPHOLOGY_VERSION_H5_1)

    assert_equal(len(n.sections), 84)
    assert_equal(len(n.soma.points), 3)
    assert_equal(len(list(n.iter())), 84)
    assert_equal(len(n.points), 924)

    section_types = list(s.type for s in n.iter())
    assert_equal(len(section_types), 84)
    real_section_types = list(chain(repeat(SectionType.apical_dendrite, 21),
                                    repeat(SectionType.basal_dendrite, 42),
                                    repeat(SectionType.axon, 21)))

    assert_equal(section_types, real_section_types)
    assert_array_equal(n.points[:7],
                       [[0.0, 0.0, 0.0],
                        [0.0, 0.0, 0.10000000149011612],
                        [0.5529246926307678, -0.7534923553466797, 0.9035181403160095],
                        [1.2052767276763916, -1.3861794471740723, 1.6835479736328125],
                        [1.2670834064483643, -1.3914604187011719, 2.4186644554138184],
                        [1.271288275718689, -2.3130500316619873, 3.192789077758789],
                        [1.605881929397583, -2.6893420219421387, 3.992844343185425]])


def test_wrong_section_type():
    with assert_raises(RawDataError) as cm:
        Morphology(os.path.join(H5V1_PATH, 'simple-broken-section-type.h5'))
    assert_equal(str(cm.exception).strip(), 'Unsupported section type: 9')

    with assert_raises(RawDataError) as cm:
        Morphology(os.path.join(H5V1_PATH, 'simple-negative-section-type.h5'))
    assert_equal(str(cm.exception).strip(), 'Unsupported section type: -2')

def test_v2():
    n = Morphology(os.path.join(H5V2_PATH, 'Neuron.h5'))
    assert_equal(n.version, MorphologyVersion.MORPHOLOGY_VERSION_H5_2)


    # This is a bug in the Neuron.h5 file
    # First neurite should not have a type: soma
    assert_equal(n.root_sections[0].type, 1)
    assert_equal(n.root_sections[1].type, 4)
    assert_equal(len(list(n.iter())), 85)
    assert_equal(len(n.points), 926)
    assert_equal(len(n.sections), 85)

def test_soma_no_neurite():
    n = Morphology(os.path.join(H5V1_PATH, 'soma_no_neurites.h5'))

    assert_array_equal(n.soma.points,
                       [[ 0.,  0.,  0.],
                        [ 1.,  0.,  0.],
                        [ 0., 25.,  0.]])
    assert_array_equal(n.soma.diameters,
                       [6,6,15])

    assert_equal(len(n.root_sections), 0)


def test_single_children():
    with captured_output() as (_, _):
        with ostream_redirect(stdout=True, stderr=True):
            neuron = Morphology(os.path.join(H5V1_PATH, 'two_child_unmerged.h5'))
    assert_equal(len(list(neuron.iter())), 3)
