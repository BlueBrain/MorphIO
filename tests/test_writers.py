import os
import numpy as np
from numpy.testing import assert_array_equal
from nose.tools import assert_equal, ok_

from morphio.mut import Morphology
from morphio import SectionType, PointLevel, MitochondriaPointLevel, Morphology as ImmutMorphology, ostream_redirect

from utils import captured_output

_path = os.path.dirname(os.path.abspath(__file__))


def test_write_soma_basic():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [0, 0, 0], [0, 0, 0]]
    morpho.soma.diameters = [2, 3, 3]

    morpho.write("test_write.swc")


def test_write_basic():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    dendrite = morpho.append_section(-1,
                                     PointLevel([[0, 0, 0],
                                                 [0, 5, 0]],
                                                [2, 2]),
                                     SectionType.basal_dendrite)

    morpho.append_section(dendrite,
                          PointLevel([[0, 5, 0],
                                      [-5, 5, 0]],
                                     [2, 3]))

    morpho.append_section(dendrite,
                          PointLevel([[0, 5, 0],
                                      [6, 5, 0]],
                                     [2, 3]))

    axon = morpho.append_section(-1,
                                 PointLevel([[0, 0, 0],
                                             [0, -4, 0]],
                                            [2, 2]),
                                 SectionType.axon)

    morpho.append_section(axon,
                          PointLevel([[0, -4, 0],
                                      [6, -4, 0]],
                                     [2, 4]))

    axon = morpho.append_section(axon,
                                 PointLevel([[0, -4, 0],
                                             [-5, -4, 0]],
                                            [2, 4]))

    morpho.write("test_write.asc")
    morpho.write("test_write.swc")
    morpho.write("test_write.h5")

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
                                     PointLevel([[0, 0, 0],
                                                 [0, 5, 0]],
                                                [2, 2],
                                                [5, 6]),
                                     SectionType.basal_dendrite)

    morpho.append_section(dendrite,
                          PointLevel([[0, 5, 0],
                                      [-5, 5, 0]],
                                     [2, 3],
                                     [6, 7]))

    morpho.append_section(dendrite,
                          PointLevel([[0, 5, 0],
                                      [6, 5, 0]],
                                     [2, 3],
                                     [6, 8]))

    morpho.write("test_write.h5")

    assert_equal(ImmutMorphology(morpho), ImmutMorphology("test_write.h5"))


def test_mitochondria():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [1, 1, 1]]
    morpho.soma.diameters = [1, 1]

    section_id = morpho.append_section(
        -1, PointLevel([[2, 2, 2], [3, 3, 3]], [4, 4], [5, 5]),
        SectionType.axon,)

    neuronal_section_ids = [0, 0]
    relative_pathlengths = np.array([0.5, 0.6], dtype=np.float32)
    diameters = [10, 20]
    mito_id = morpho.mitochondria.append_section(
        -1, MitochondriaPointLevel(neuronal_section_ids,
                                   relative_pathlengths,
                                   diameters))

    morpho.mitochondria.append_section(
        mito_id, MitochondriaPointLevel([0, 0, 0, 0],
                                        [0.6, 0.7, 0.8, 0.9],
                                        [20, 30, 40, 50]))
    morpho.write("test.h5")

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morpho.write("test.swc")
            assert_equal(err.getvalue().strip(),
                         "This cell has mitochondria, they cannot be saved in  ASC or SWC format. Please use H5 if you want to save them.")

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morpho.write("test.asc")
            assert_equal(err.getvalue().strip(),
                         "This cell has mitochondria, they cannot be saved in  ASC or SWC format. Please use H5 if you want to save them.")

    mito = ImmutMorphology('test.h5').mitochondria
    assert_array_equal(mito.root_sections[0].diameters,
                       diameters)
    assert_array_equal(mito.root_sections[0].neurite_section_ids,
                       neuronal_section_ids)
    assert_array_equal(mito.root_sections[0].relative_path_lengths,
                       relative_pathlengths)

    assert_equal(len(mito.root_sections), 1)

    mito = Morphology('test.h5').mitochondria
    assert_equal(mito.root_sections, [0])
    assert_array_equal(mito.section(0).diameters,
                       diameters)

    assert_array_equal(mito.section(0).neurite_section_ids,
                       neuronal_section_ids)
