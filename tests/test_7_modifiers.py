# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path

import numpy as np
from morphio import Collection, Morphology, Option, SectionType, ostream_redirect
from morphio.mut import Morphology as MutableMorphology
from numpy.testing import assert_array_equal
from utils import captured_output


DATA_DIR = Path(__file__).parent / "data"
SIMPLE = DATA_DIR /  'simple.swc'

SIMPLE_NO_MODIFIER = Morphology(SIMPLE)


def test_no_modifier():
    assert_array_equal(SIMPLE_NO_MODIFIER.points,
                       Morphology(SIMPLE, options=Option.no_modifier).points)


def check_nrn_order(m):
    assert ([section.type for section in m.root_sections] ==
                 [SectionType.axon,
                  SectionType.basal_dendrite,
                  SectionType.apical_dendrite])


def test_nrn_order():
    morph_name = 'reversed_NRN_neurite_order'
    filename = DATA_DIR /  f'{morph_name}.swc'

    check_nrn_order(Morphology(filename, options=Option.nrn_order))

    normal = Morphology(filename)
    check_nrn_order(MutableMorphology(normal, options=Option.nrn_order))

    normal = MutableMorphology(filename)
    check_nrn_order(MutableMorphology(normal, options=Option.nrn_order))

    collection_relpaths = ["h5/v1", "h5/v1/merged.h5"]
    for collection_relpath in collection_relpaths:
        collection = Collection(DATA_DIR /  collection_relpath)

        check_nrn_order(collection.load(morph_name, options=Option.nrn_order))
        check_nrn_order(collection.load(morph_name, options=Option.nrn_order, mutable=True))

        for _, morph in collection.load_unordered([morph_name], options=Option.nrn_order):
            check_nrn_order(morph)

        for _, morph in collection.load_unordered([morph_name],
                                                  options=Option.nrn_order,
                                                  mutable=True):
            check_nrn_order(morph)


def test_nrn_order_stability():
    m = Morphology(DATA_DIR /  'mono-type.asc', Option.nrn_order)
    assert_array_equal([root.points[0, 0] for root in m.root_sections
                        if root.type == SectionType.basal_dendrite], [0, 1, 2, 3])
    assert_array_equal([root.points[0, 0] for root in m.root_sections
                        if root.type == SectionType.axon], [0, 1])
    assert_array_equal([root.points[0, 0] for root in m.root_sections
                        if root.type == SectionType.apical_dendrite], [1, 0])

    # This morphology used to give a different order when opened with nrn_order flag
    filename = DATA_DIR /  'nrn-order-already-sorted.h5'
    a = Morphology(filename, Option.nrn_order)
    b = Morphology(filename)
    for sec1, sec2 in zip(a.root_sections, b.root_sections):
        assert_array_equal(sec1.points, sec2.points)



def test_two_point_section():
    m = Morphology(DATA_DIR /  'multiple_point_section.asc', options=Option.two_points_sections)
    assert_array_equal([section.points for section in m.iter()],
                       [[[0.,0.,0.], [10.,50.,0.]],
                        [[10,50,0], [0,1,2]],
                        [[10,50,0], [0,4,5]],
                        [[0,0,0], [7,7,7]]])

def test_soma_sphere():
    m = Morphology(DATA_DIR /  'soma_multiple_frustums.swc', options=Option.soma_sphere)
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
