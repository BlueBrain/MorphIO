import os

import numpy as np
from morphio import Morphology, Option, SectionType, ostream_redirect
from morphio.mut import Morphology as MutableMorphology
from nose.tools import assert_equal
from numpy.testing import assert_array_equal

from .utils import captured_output

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
SIMPLE = os.path.join(_path, 'simple.swc')

SIMPLE_NO_MODIFIER = Morphology(SIMPLE)

def test_no_modifier():
    assert_array_equal(SIMPLE_NO_MODIFIER.points, Morphology(SIMPLE, options=Option.no_modifier).points)

def test_nrn_order():
    filename = os.path.join(_path, 'reversed_NRN_neurite_order.swc')

    m = Morphology(filename, options=Option.nrn_order)
    assert_equal([section.type for section in m.root_sections],
                 [SectionType.axon,
                  SectionType.basal_dendrite,
                  SectionType.apical_dendrite])

    normal = Morphology(filename)
    m = MutableMorphology(normal, options=Option.nrn_order)
    assert_equal([section.type for section in m.root_sections],
                 [SectionType.axon,
                  SectionType.basal_dendrite,
                  SectionType.apical_dendrite])

    normal = MutableMorphology(filename)
    m = MutableMorphology(normal, options=Option.nrn_order)
    assert_equal([section.type for section in m.root_sections],
                 [SectionType.axon,
                  SectionType.basal_dendrite,
                  SectionType.apical_dendrite])


def test_nrn_order_stability():
    m = Morphology(os.path.join(_path, 'mono-type.asc'), Option.nrn_order)
    assert_array_equal([root.points[0, 0] for root in m.root_sections
                        if root.type == SectionType.basal_dendrite], [0, 1, 2, 3])
    assert_array_equal([root.points[0, 0] for root in m.root_sections
                        if root.type == SectionType.axon], [0, 1])
    assert_array_equal([root.points[0, 0] for root in m.root_sections
                        if root.type == SectionType.apical_dendrite], [1, 0])

    # This morphology used to give a different order when opened with nrn_order flag
    filename = os.path.join(_path, 'nrn-order-already-sorted.h5')
    a = Morphology(filename, Option.nrn_order)
    b = Morphology(filename)
    for sec1, sec2 in zip(a.root_sections, b.root_sections):
        assert_array_equal(sec1.points, sec2.points)



def test_two_point_section():
    m = Morphology(os.path.join(_path, 'multiple_point_section.asc'), options=Option.two_points_sections)
    assert_array_equal([section.points for section in m.iter()],
                       [[[0.,0.,0.], [10.,50.,0.]],
                        [[10,50,0], [0,1,2]],
                        [[10,50,0], [0,4,5]],
                        [[0,0,0], [7,7,7]]])

def test_soma_sphere():
    m = Morphology(os.path.join(_path, 'soma_multiple_frustums.swc'), options=Option.soma_sphere)
    assert_array_equal(m.soma.points,
                       [[1.5, 0, 0]])

def test_no_duplicate():
    with captured_output():
        with ostream_redirect(stdout=True, stderr=True):
            m = Morphology(SIMPLE, options=Option.no_duplicates)

    neurite1 = np.array([[0.,0.,0.], [0,5,0],
                         [-5,5,0],
                         [6,5,0]])

    neurite2 = np.array([[0,0,0], [0,-4,0],
                [6,-4,0],
                [-5,-4,0]])

    assert_array_equal(np.vstack([section.points for section in m.iter()]),
                       np.vstack([neurite1, neurite2]))

    # Combining options NO_DUPLICATES and NRN_ORDER
    with captured_output():
        with ostream_redirect(stdout=True, stderr=True):
            m = Morphology(SIMPLE, options=Option.no_duplicates|Option.nrn_order)
    assert_array_equal(np.vstack([section.points for section in m.iter()]),
                       np.vstack([neurite2, neurite1]))
