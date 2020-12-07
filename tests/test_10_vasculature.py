import os

import numpy as np
from nose.tools import assert_equal, assert_raises
from numpy.testing import assert_array_almost_equal, assert_array_equal
from pathlib import Path

import morphio.vasculature as vasculature
from morphio import RawDataError, VasculatureSectionType

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


def test_empty_vasculature():
    assert_raises(RawDataError, vasculature.Vasculature, os.path.join(_path, "h5/empty_vasculature.h5"))


def test_components_vasculature():
    morphology = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
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

    assert_equal(len(morphology.sections), 3080)
    assert_equal(len(morphology.points), 55807)
    assert_equal(len(morphology.diameters), 55807)
    assert_array_almost_equal(morphology.diameters[-5:],
                              np.array([0.78039801, 0.78039801, 0.78039801, 2.11725187, 2.11725187]))
    assert_equal(len(morphology.section_types), 3080)
    assert_equal(len(morphology.section(0).predecessors), 0)
    assert_equal(len(morphology.section(0).successors), 2)

    assert_equal(morphology.section(0).successors[0].id, 1)
    assert_equal(morphology.section(0).successors[1].id, 2)


def test_section_types():
    morphology = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
    assert_equal(morphology.section(0).type, VasculatureSectionType.vein)
    assert_equal(morphology.section(1).type, VasculatureSectionType.artery)
    assert_equal(morphology.section(2).type, VasculatureSectionType.venule)
    assert_equal(morphology.section(3).type, VasculatureSectionType.arteriole)
    assert_equal(morphology.section(4).type, VasculatureSectionType.venous_capillary)
    assert_equal(morphology.section(5).type, VasculatureSectionType.arterial_capillary)

    assert_raises(RawDataError, vasculature.Vasculature, os.path.join(_path, "h5/vasculature-broken-section-type.h5"))


def test_iterators_vasculature():
    morphology = vasculature.Vasculature(os.path.join(_path, "h5/vasculature1.h5"))
    assert_array_equal([sec.id for sec in morphology.sections], range(3080))
    assert_equal(len([section.id for section in morphology.iter()]), 3080)
    all_sections = set([sec.id for sec in morphology.sections])
    for sec in morphology.iter():
        all_sections.remove(sec.id)
    assert_equal(len(all_sections), 0)


def test_from_pathlib():
    vasc = vasculature.Vasculature(Path(_path, "h5/vasculature1.h5"))
    assert_equal(len(vasc.sections), 3080)
