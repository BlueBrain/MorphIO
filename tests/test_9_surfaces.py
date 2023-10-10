# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
from pathlib import Path

import numpy as np
import pytest
from morphio import Morphology as ImmutMorphology
from morphio import SomaError
from morphio.mut import Morphology
from numpy.testing import assert_almost_equal


DATA_DIR = Path(__file__).parent / "data"


def test_contour_surface():
    # # circle contour
    # assert_almost_equal(ImmutMorphology(DATA_DIR /  "circle_contour.asc").soma.surface,
    #                     np.pi, decimal=2)
    # assert_almost_equal(Morphology(DATA_DIR /  "circle_contour.asc").soma.surface,
    #                     np.pi, decimal=2)

    # # triangular contour
    # assert_almost_equal(ImmutMorphology(DATA_DIR /  "h5/v1/Neuron.h5").soma.surface,
    #                     0.01, decimal=5)
    # assert_almost_equal(Morphology(DATA_DIR /  "h5/v1/Neuron.h5").soma.surface,
    #                     0.01, decimal=5)

    # # single point ASC
    # assert_almost_equal(ImmutMorphology(DATA_DIR /  "simple.asc").soma.surface,
    #                     4 * np.pi, decimal=2)
    # assert_almost_equal(Morphology(DATA_DIR /  "simple.asc").soma.surface,
    #                     4 * np.pi, decimal=2)

    # single point SWC
    assert_almost_equal(ImmutMorphology(DATA_DIR /  "simple.swc").soma.surface,
                        4 * np.pi, decimal=2)
    assert_almost_equal(Morphology(DATA_DIR /  "simple.swc").soma.surface,
                        4 * np.pi, decimal=2)

    # single point H5
    assert_almost_equal(ImmutMorphology(DATA_DIR /  "h5/v1/simple.h5").soma.surface,
                        4 * np.pi, decimal=2)
    assert_almost_equal(Morphology(DATA_DIR /  "h5/v1/simple.h5").soma.surface,
                        4 * np.pi, decimal=2)

    # SWC three points cylinder
    assert_almost_equal(ImmutMorphology(DATA_DIR /  "soma_three_points_cylinder.swc").soma.surface,
                        4 * np.pi * 81, decimal=2)
    assert_almost_equal(Morphology(DATA_DIR /  "soma_three_points_cylinder.swc").soma.surface,
                        4 * np.pi * 81, decimal=2)

    # SWC consecutive cylinders  (3 cylinders along X)
    assert_almost_equal(ImmutMorphology(DATA_DIR /  "soma_cylinders.swc").soma.surface,
                        2 * np.pi * 40 * 3, decimal=2)
    assert_almost_equal(Morphology(DATA_DIR /  "soma_cylinders.swc").soma.surface,
                        2 * np.pi * 40 * 3, decimal=2)

    with pytest.raises(SomaError):
        Morphology(DATA_DIR /  "no_soma.swc").soma.surface

    assert_almost_equal(Morphology(DATA_DIR /  "soma_single_frustum.swc").soma.surface,
                        1201.428, decimal=3)

    # SWC multiple frustums
    assert_almost_equal(ImmutMorphology(DATA_DIR /  "soma_multiple_frustums.swc").soma.surface,
                        4164.610254415956, decimal=3)
    assert_almost_equal(Morphology(DATA_DIR /  "soma_multiple_frustums.swc").soma.surface,
                        4164.610254415956, decimal=3)

    # SWC complex
    assert_almost_equal(ImmutMorphology(DATA_DIR /  "complexe.swc").soma.surface,
                        13.980, decimal=2)
