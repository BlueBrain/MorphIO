"""
Readonly Morphology returns readonly numpy views (not copies) to its internal
points, diameters and perimeters datasets. Here it is ensures that refcounting
of the Morphology is increased when these views are available on the python side.
Morphology can be released only when all references to itself and its views are
freed. The standard datasets of all the readonly morphologies are:
    - points
    - diameters
    - section_types
"""
import sys
import pytest
import morphio
from pathlib import Path

import numpy as np
from numpy import testing as npt

DATA_DIR = Path(__file__).parent.resolve() / "data"

READONLY_CLASSES_PATHS = [
    (morphio.Morphology, DATA_DIR / "simple-heterogeneous-neurite.swc"),
    (morphio.GlialCell, DATA_DIR / "astrocyte.h5"),
    (morphio.vasculature.Vasculature, DATA_DIR  / "h5/vasculature1.h5"),
    (morphio.DendriticSpine, DATA_DIR / "h5/v1/simple-dendritric-spine.h5")
]


def _assert_refcount(obj, expected_refcount):
    actual_refcount = sys.getrefcount(obj) - 3
    assert actual_refcount == expected_refcount, (
        f"\nActual: {actual_refcount}\n"
        f"Expected: {expected_refcount}"
    )


@pytest.mark.parametrize("MorphologyClass, path", READONLY_CLASSES_PATHS)
def test_morphology_level_immutability(MorphologyClass, path):

    morph = MorphologyClass(path)

    # ensure we cannot overwrite the attributes
    with pytest.raises(AttributeError):
        morph.points = np.ones(morph.points.shape)

    with pytest.raises(AttributeError):
        morph.diameters = np.ones(morph.diameters.shape)

    with pytest.raises(AttributeError):
        morph.section_types = np.ones(morph.section_types.shape)

    # ensure we cannot change the underlying data
    with pytest.raises(ValueError):
        morph.points[:] = np.ones(morph.points.shape)

    with pytest.raises(ValueError):
        morph.diameters[:] = np.ones(morph.diameters.shape)

    with pytest.raises(ValueError):
        morph.section_types[:] = np.ones(morph.section_types.shape)

    # check if flag set correctly
    assert not morph.points.flags["WRITEABLE"]
    assert not morph.diameters.flags["WRITEABLE"]
    assert not morph.section_types.flags["WRITEABLE"]

    # check that the parent of the array is the morphology
    assert morph.points.base is morph
    assert morph.diameters.base is morph
    assert morph.section_types.base is morph

    # make the same tests when the datasets are assigned to a variable
    points = morph.points
    _assert_refcount(points, 1)
    assert points.base is morph
    assert points.flags["WRITEABLE"] == False

    diameters = morph.diameters
    _assert_refcount(diameters, 1)
    assert diameters.base is morph
    assert diameters.flags["WRITEABLE"] == False

    section_types = morph.section_types
    _assert_refcount(section_types, 1)
    assert section_types.base is morph
    assert section_types.flags["WRITEABLE"] == False

import gc
@pytest.mark.parametrize("MorphologyClass, path", READONLY_CLASSES_PATHS)
def test_section_level_immutability(MorphologyClass, path):

    morph = MorphologyClass(path)
    _assert_refcount(morph, 1)

    for section in morph.iter():

        _assert_refcount(section, 1)
        _assert_refcount(morph, 2)

        points = section.points
        assert points.base is section
        _assert_refcount(section, 2)
        _assert_refcount(morph, 2)

        diameters = section.diameters
        assert diameters.base is section
        _assert_refcount(section, 3)
        _assert_refcount(morph, 2)


@pytest.mark.parametrize("MorphologyClass, path", READONLY_CLASSES_PATHS)
def test_copies_mutability(MorphologyClass, path):

    morph = MorphologyClass(path)

    _assert_refcount(morph, 1)

    points_copy = morph.points.copy()
    diameters_copy = morph.diameters.copy()
    section_types_copy = morph.section_types.copy()

    # we copy the points, morph refcount should not change
    _assert_refcount(morph, 1)

    # sanity check
    npt.assert_array_almost_equal(points_copy, morph.points)
    npt.assert_array_almost_equal(diameters_copy, morph.diameters)
    npt.assert_array_almost_equal(section_types_copy, morph.section_types)

    # check that new datasets are writeable
    new_points = morph.points + np.zeros(morph.points.shape)
    assert new_points.flags["WRITEABLE"]

    new_diameters = morph.diameters + np.zeros(morph.diameters.shape)
    assert new_diameters.flags["WRITEABLE"]

    new_section_types = morph.section_types + np.zeros(morph.section_types.shape)
    assert new_section_types.flags["WRITEABLE"]


@pytest.mark.parametrize("MorphologyClass, path", READONLY_CLASSES_PATHS)
def test_morphology_refcount(MorphologyClass, path):
    morph = MorphologyClass(path)

    points = morph.points
    diameters = morph.diameters
    section_types = morph.section_types

    # morph refcount should be 1 (morph) + 3 (points, diameters, section_types)
    _assert_refcount(morph, 4)

    # removing the points, should decrease morph refcount by 1
    del points
    _assert_refcount(morph, 3)

    # removing the diameters, should decrease morph refcount by 1
    del diameters
    _assert_refcount(morph, 2)

    del section_types
    _assert_refcount(morph, 1)


@pytest.mark.parametrize("MorphologyClass, path", READONLY_CLASSES_PATHS)
def test_morphology_not_released_while_datasets_in_scope(MorphologyClass, path):

    morph = MorphologyClass(path)

    # morphology should not be released if it's del/outscoped in python
    points = morph.points
    diameters = morph.diameters
    section_types = morph.section_types

    points_copy = points.copy()
    diameters_copy = diameters.copy()
    section_types_copy = section_types.copy()

    del morph
    _assert_refcount(points, 1)
    npt.assert_array_almost_equal(points_copy, points)
    del points

    _assert_refcount(diameters, 1)
    npt.assert_array_almost_equal(diameters_copy, diameters)
    del diameters

    _assert_refcount(section_types, 1)
    npt.assert_array_almost_equal(section_types_copy, section_types)
