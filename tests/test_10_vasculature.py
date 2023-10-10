# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path

import h5py
import numpy as np
import pytest
from numpy import testing as npt
from numpy.testing import assert_array_almost_equal, assert_array_equal

import morphio.vasculature as vasculature
from morphio import RawDataError, VasculatureSectionType


DATA_DIR = Path(__file__).parent / "data"


def test_empty_vasculature():
    with pytest.raises(RawDataError):
        vasculature.Vasculature(DATA_DIR /  "h5/empty_vasculature.h5")


def test_components_vasculature():
    morphology = vasculature.Vasculature(DATA_DIR /  "h5/vasculature1.h5")
    assert_array_almost_equal(morphology.section(0).points,
                              np.array([[1265.47399902,  335.42364502, 1869.19274902],
                                        [1266.26647949,  335.57000732, 1869.74914551],
                                        [1267.09082031,  335.68869019, 1870.31469727],
                                        [1267.89404297,  335.78134155, 1870.91418457],
                                        [1268.67077637,  335.85733032, 1871.54992676],
                                        [1269.42773438,  335.92602539, 1872.21008301],
                                        [1270.17431641,  335.99368286, 1872.88195801],
                                        [1270.92016602,  336.06558228, 1873.55395508],
                                        [1271.6739502 ,  336.14227295, 1874.21740723],
                                        [1272.44091797,  336.23706055, 1874.86047363],
                                        [1273.22216797,  336.31613159, 1875.49523926],
                                        [1274.        ,  336.70001221, 1876.        ]]))

    assert_array_almost_equal(morphology.section(0).diameters,
                              np.array([1.96932483, 1.96932483, 1.96932483, 1.96932483, 1.96932483,
                                        1.96932483, 1.96932483, 1.96932483, 1.96932483, 1.96932483,
                                        1.96932483, 2.15068388]))

    assert len(morphology.sections) == 3080
    assert len(morphology.points) == 55807
    assert morphology.n_points == len(morphology.points)
    assert len(morphology.diameters) == 55807
    assert_array_almost_equal(
        morphology.diameters[-5:],
        np.array([0.78039801, 0.78039801, 0.78039801, 2.11725187, 2.11725187]))
    assert len(morphology.section_types) == 3080
    assert morphology.section(0).n_points == len(morphology.section(0).points)
    assert len(morphology.section(0).predecessors) == 0
    assert len(morphology.section(0).successors) == 2

    assert morphology.section(0).successors[0].id == 1
    assert morphology.section(0).successors[1].id == 2


def test_section_types():
    morphology = vasculature.Vasculature(DATA_DIR /  "h5/vasculature1.h5")
    assert morphology.section(0).type == VasculatureSectionType.vein
    assert morphology.section(1).type == VasculatureSectionType.artery
    assert morphology.section(2).type == VasculatureSectionType.venule
    assert morphology.section(3).type == VasculatureSectionType.arteriole
    assert morphology.section(4).type == VasculatureSectionType.venous_capillary
    assert morphology.section(5).type == VasculatureSectionType.arterial_capillary

    with pytest.raises(RawDataError):
        vasculature.Vasculature(DATA_DIR /  "h5/vasculature-broken-section-type.h5")


def test_section_offsets():
    """
    Example:

    Array of points (listing ids instead of coordinates)
    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

    And three sections:

    S1: [0, 1, 2]
    S2: [3, 4, 5]
    S3: [6, 7, 8, 9]

    The offsets will be:
    [0, 3, 6, 10]

    So the last entry is equal to the size of the points dataset. This allows for applying a diff
    for instance to calculate the section length (in terms of points) for each section without
    having to allocate an extra array to add that extra value.
    """
    morphology = vasculature.Vasculature(DATA_DIR /  "h5/vasculature1.h5")

    offset = 0
    expected_offsets = [offset]
    expected_lengths = []

    for sec in morphology.sections:

        ps = sec.points
        offset += len(ps)
        expected_lengths.append(len(ps))
        expected_offsets.append(offset)

    actual_section_offsets = morphology.section_offsets
    npt.assert_allclose(actual_section_offsets, expected_offsets)

    # last entry in the section offsets should be equal to the total
    # number of points`
    npt.assert_equal(actual_section_offsets[-1], len(morphology.points))

    # and from the offsets we calculate the number of points per section
    section_lengths = np.diff(actual_section_offsets)
    npt.assert_allclose(section_lengths, expected_lengths)


def test_section_connectivity():

    path = DATA_DIR /  "h5/vasculature1.h5"
    morphology = vasculature.Vasculature(path)

    with h5py.File(path, 'r') as fd:
        expected_connectivity = fd['connectivity'][:]

    assert_array_almost_equal(
        morphology.section_connectivity,
        expected_connectivity
    )


def test_iterators_vasculature():
    morphology = vasculature.Vasculature(DATA_DIR /  "h5/vasculature1.h5")
    assert_array_equal([sec.id for sec in morphology.sections], range(3080))
    assert len([section.id for section in morphology.iter()]) == 3080
    all_sections = set([sec.id for sec in morphology.sections])
    for sec in morphology.iter():
        all_sections.remove(sec.id)
    assert len(all_sections) == 0


def test_from_pathlib():
    vasc = vasculature.Vasculature(DATA_DIR / "h5/vasculature1.h5")
    assert len(vasc.sections) == 3080

def test_section___str__():
    morphology = vasculature.Vasculature(DATA_DIR /  "h5/vasculature1.h5")
    assert (str(morphology.section(0)) ==
                 'Section(id=0, points=[(1265.47 335.424 1869.19),..., (1274 336.7 1876)])')
