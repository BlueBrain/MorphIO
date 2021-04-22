import os
import numpy as np
from numpy.testing import assert_almost_equal
import pytest

from morphio import Morphology as ImmutMorphology, SomaError
from morphio.mut import Morphology

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


def test_contour_surface():
    # # circle contour
    # assert_almost_equal(ImmutMorphology(os.path.join(_path, "circle_contour.asc")).soma.surface,
    #                     np.pi, decimal=2)
    # assert_almost_equal(Morphology(os.path.join(_path, "circle_contour.asc")).soma.surface,
    #                     np.pi, decimal=2)

    # # triangular contour
    # assert_almost_equal(ImmutMorphology(os.path.join(_path, "h5/v1/Neuron.h5")).soma.surface,
    #                     0.01, decimal=5)
    # assert_almost_equal(Morphology(os.path.join(_path, "h5/v1/Neuron.h5")).soma.surface,
    #                     0.01, decimal=5)

    # # single point ASC
    # assert_almost_equal(ImmutMorphology(os.path.join(_path, "simple.asc")).soma.surface,
    #                     4 * np.pi, decimal=2)
    # assert_almost_equal(Morphology(os.path.join(_path, "simple.asc")).soma.surface,
    #                     4 * np.pi, decimal=2)

    # single point SWC
    assert_almost_equal(ImmutMorphology(os.path.join(_path, "simple.swc")).soma.surface,
                        4 * np.pi, decimal=2)
    assert_almost_equal(Morphology(os.path.join(_path, "simple.swc")).soma.surface,
                        4 * np.pi, decimal=2)

    # single point H5
    assert_almost_equal(ImmutMorphology(os.path.join(_path, "h5/v1/simple.h5")).soma.surface,
                        4 * np.pi, decimal=2)
    assert_almost_equal(Morphology(os.path.join(_path, "h5/v1/simple.h5")).soma.surface,
                        4 * np.pi, decimal=2)

    # SWC three points cylinder
    assert_almost_equal(ImmutMorphology(os.path.join(_path, "soma_three_points_cylinder.swc")).soma.surface,
                        4 * np.pi * 81, decimal=2)
    assert_almost_equal(Morphology(os.path.join(_path, "soma_three_points_cylinder.swc")).soma.surface,
                        4 * np.pi * 81, decimal=2)

    # SWC consecutive cylinders  (3 cylinders along X)
    assert_almost_equal(ImmutMorphology(os.path.join(_path, "soma_cylinders.swc")).soma.surface,
                        2 * np.pi * 40 * 3, decimal=2)
    assert_almost_equal(Morphology(os.path.join(_path, "soma_cylinders.swc")).soma.surface,
                        2 * np.pi * 40 * 3, decimal=2)

    with pytest.raises(SomaError):
        _ = Morphology(os.path.join(_path, "no_soma.swc")).soma.surface

    assert_almost_equal(Morphology(os.path.join(_path, "soma_single_frustum.swc")).soma.surface,
                        1201.428, decimal=3)
    # SWC multiple frustums
    assert_almost_equal(ImmutMorphology(os.path.join(_path, "soma_multiple_frustums.swc")).soma.surface,
                        4164.610254415956, decimal=3)
    assert_almost_equal(Morphology(os.path.join(_path, "soma_multiple_frustums.swc")).soma.surface,
                        4164.610254415956, decimal=3)

    # SWC complex
    assert_almost_equal(ImmutMorphology(os.path.join(_path, "complexe.swc")).soma.surface,
                        13.980, decimal=2)
