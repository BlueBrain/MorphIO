import os
from numpy.testing import assert_array_equal
from nose.tools import assert_equal, assert_raises, ok_

from morphio.mut import Morphology
from morphio import SectionType, PointLevel, Morphology as ImmutMorphology

_path = os.path.dirname(os.path.abspath(__file__))


def test_write_basic():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    dendrite = morpho.append_section(-1,
                                     SectionType.basal_dendrite,
                                     PointLevel([[0, 0, 0],
                                                 [0, 5, 0]],
                                                [2, 2]))

    morpho.append_section(dendrite,
                          SectionType.basal_dendrite,
                          PointLevel([[0, 5, 0],
                                      [-5, 5, 0]],
                                     [2, 3]))

    morpho.append_section(dendrite,
                          SectionType.basal_dendrite,
                          PointLevel([[0, 5, 0],
                                      [6, 5, 0]],
                                     [2, 3]))

    axon = morpho.append_section(-1,
                                 SectionType.axon,
                                 PointLevel([[0, 0, 0],
                                             [0, -4, 0]],
                                            [2, 2]))

    morpho.append_section(axon,
                          SectionType.axon,
                          PointLevel([[0, -4, 0],
                                      [6, -4, 0]],
                                     [2, 4]))

    axon = morpho.append_section(axon,
                                 SectionType.axon,
                                 PointLevel([[0, -4, 0],
                                             [-5, -4, 0]],
                                            [2, 4]))

    morpho.write_asc("test_write.asc")
    morpho.write_swc("test_write.swc")
    morpho.write_h5("test_write.h5")

    assert_equal(ImmutMorphology(morpho), ImmutMorphology("test_write.asc"))
    assert_equal(ImmutMorphology(morpho), ImmutMorphology("test_write.swc"))
    assert_equal(ImmutMorphology(morpho), ImmutMorphology("test_write.h5"))
    assert_equal(ImmutMorphology(morpho), ImmutMorphology(os.path.join(_path, "simple.asc")))
    ok_(not (ImmutMorphology(morpho) != ImmutMorphology(os.path.join(_path, "simple.asc"))))


def test_write_perimeter():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    dendrite = morpho.append_section(-1,
                                     SectionType.basal_dendrite,
                                     PointLevel([[0, 0, 0],
                                                 [0, 5, 0]],
                                                [2, 2],
                                                [5, 6]))

    morpho.append_section(dendrite,
                          SectionType.basal_dendrite,
                          PointLevel([[0, 5, 0],
                                      [-5, 5, 0]],
                                     [2, 3],
                                     [6, 7]))

    morpho.append_section(dendrite,
                          SectionType.basal_dendrite,
                          PointLevel([[0, 5, 0],
                                      [6, 5, 0]],
                                     [2, 3],
                                     [6, 8]))

    morpho.write_h5("test_write.h5")

    assert_equal(ImmutMorphology(morpho), ImmutMorphology("test_write.h5"))
