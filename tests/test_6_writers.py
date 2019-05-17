import os
import numpy as np
from numpy.testing import assert_array_equal, assert_equal, assert_raises
from nose.tools import ok_

from morphio.mut import Morphology
from morphio import (SectionBuilderError, set_maximum_warnings, SectionType, PointLevel,
                     MitochondriaPointLevel, Morphology as ImmutMorphology, ostream_redirect)

from utils import captured_output, setup_tempdir

_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")


def test_write_soma_basic():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [0, 0, 0], [0, 0, 0]]
    morpho.soma.diameters = [2, 3, 3]

    with setup_tempdir('test_write_soma_basic') as tmp_folder:
        morpho.write(os.path.join(tmp_folder, "test_write.swc"))


def test_write_basic():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    dendrite = morpho.append_root_section(PointLevel([[0, 0, 0], [0, 5, 0]], [2, 2]),
                                          SectionType.basal_dendrite)

    dendrite.append_section(PointLevel([[0, 5, 0], [-5, 5, 0]], [2, 3]))
    dendrite.append_section(PointLevel([[0, 5, 0], [6, 5, 0]], [2, 3]))

    axon = morpho.append_root_section(
                                 PointLevel([[0, 0, 0],
                                             [0, -4, 0]],
                                            [2, 2]),
                                 SectionType.axon)

    axon.append_section(PointLevel([[0, -4, 0],
                                      [6, -4, 0]],
                                     [2, 4]))

    axon = axon.append_section(PointLevel([[0, -4, 0],
                                             [-5, -4, 0]],
                                            [2, 4]))

    with setup_tempdir('test_write_basic') as tmp_folder:
        morpho.write(os.path.join(tmp_folder, "test_write.asc"))
        morpho.write(os.path.join(tmp_folder, "test_write.swc"))
        morpho.write(os.path.join(tmp_folder, "test_write.h5"))

        assert_equal(ImmutMorphology(morpho), ImmutMorphology(os.path.join(tmp_folder, "test_write.asc")))
        assert_equal(ImmutMorphology(morpho), ImmutMorphology(os.path.join(tmp_folder, "test_write.swc")))
        assert_equal(ImmutMorphology(morpho), ImmutMorphology(os.path.join(tmp_folder, "test_write.h5")))
        assert_equal(ImmutMorphology(morpho), ImmutMorphology(
            os.path.join(_path, "simple.asc")))
        ok_(not (ImmutMorphology(morpho) != ImmutMorphology(
            os.path.join(_path, "simple.asc"))))


def test_write_merge_only_child():
    '''The root section has only one child
    The child should be merged with its parent section
    Special care must be given for the potential duplicate point
                             o
                            /
                           / son 1
      root       child    /
    o--------o----------o<
                          \
                           \  son 2
                            \
                             o
    '''
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    root = morpho.append_root_section(
                                 PointLevel([[0, 0, 0],
                                             [0, 5, 0]],
                                            [2, 2]),
                                 SectionType.basal_dendrite)
    child = root.append_section(PointLevel([[0, 5, 0], [0, 6, 0]], [2, 3]))
    son1 = child.append_section(PointLevel([[0, 6, 0], [0, 7, 0]], [2, 3]))
    son2 = child.append_section(PointLevel([[0, 6, 0], [4, 5, 6]], [3, 3]))

    with setup_tempdir('test_write_merge_only_child') as tmp_folder:
        for extension in ['swc', 'asc', 'h5']:
            with captured_output() as (_, err):
                with ostream_redirect(stdout=True, stderr=True):
                    filename = os.path.join(tmp_folder, 'test.{}'.format(extension))
                    morpho.write(filename)

                    assert_equal(err.getvalue().strip(),
                                 'Section: 1 is the only child of section: 0\nIt will be merged with the parent section')


            read = Morphology(filename)
            root = read.root_sections[0]
            assert_array_equal(root.points,
                               [[0, 0, 0],
                                [0, 5, 0],
                                [0, 6, 0]])
            assert_equal(len(root.children), 2)

            assert_array_equal(root.children[0].points,
                               [[0, 6, 0], [0, 7, 0]])

            assert_array_equal(root.children[1].points,
                               [[0, 6, 0], [4, 5, 6]])



def test_write_perimeter():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    dendrite = morpho.append_root_section(
                                     PointLevel([[0, 0, 0],
                                                 [0, 5, 0]],
                                                [2, 2],
                                                [5, 6]),
                                     SectionType.basal_dendrite)

    dendrite.append_section(
                          PointLevel([[0, 5, 0],
                                      [-5, 5, 0]],
                                     [2, 3],
                                     [6, 7]))

    dendrite.append_section(
                          PointLevel([[0, 5, 0],
                                      [6, 5, 0]],
                                     [2, 3],
                                     [6, 8]))

    with setup_tempdir('test_write_perimeter') as tmp_folder:
        morpho.write(os.path.join(tmp_folder, "test_write.h5"))

        assert_array_equal(ImmutMorphology(morpho),
                           ImmutMorphology(os.path.join(tmp_folder, "test_write.h5")))


def test_write_no_soma():
    morpho = Morphology()
    dendrite = morpho.append_root_section(
                                     PointLevel([[0, 0, 0],
                                                 [0, 5, 0]],
                                                [2, 2],
                                                [5, 6]),
                                     SectionType.basal_dendrite)
    dendrite = morpho.append_root_section(
                                     PointLevel([[0, 1, 0],
                                                 [0, 7, 0]],
                                                [2, 2],
                                                [5, 6]),
                                     SectionType.basal_dendrite)

    with setup_tempdir('test_write_no_soma') as tmp_folder:
        for ext in ['asc', 'h5', 'swc']:
            with captured_output() as (_, err):
                with ostream_redirect(stdout=True, stderr=True):
                    outfile = os.path.join(tmp_folder, 'tmp.' + ext)
                    morpho.write(outfile)
                    assert_equal(err.getvalue().strip(),
                                 'Warning: writing file without a soma')

                    read = Morphology(outfile)

            assert_equal(len(read.soma.points), 0)
            assert_equal(len(read.root_sections), 2)
            assert_array_equal(read.root_sections[0].points, [[0, 0, 0], [0, 5, 0]])
            assert_array_equal(read.root_sections[1].points, [[0, 1, 0], [0, 7, 0]])


def test_mitochondria():
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [1, 1, 1]]
    morpho.soma.diameters = [1, 1]

    section_id = morpho.append_root_section(PointLevel([[2, 2, 2], [3, 3, 3]], [4, 4], [5, 5]),
        SectionType.axon,)

    neuronal_section_ids = [0, 0]
    relative_pathlengths = np.array([0.5, 0.6], dtype=np.float32)
    diameters = [10, 20]
    mito_id = morpho.mitochondria.append_root_section(
        MitochondriaPointLevel(neuronal_section_ids,
                                   relative_pathlengths,
                                   diameters))

    mito_id.append_section(
        MitochondriaPointLevel([0, 0, 0, 0],
                                        [0.6, 0.7, 0.8, 0.9],
                                        [20, 30, 40, 50]))
    with setup_tempdir('test_mitochondria') as tmp_folder:
        morpho.write(os.path.join(tmp_folder, "test.h5"))

        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                morpho.write(os.path.join(tmp_folder, "test.swc"))
                assert_equal(err.getvalue().strip(),
                             "This cell has mitochondria, they cannot be saved in  ASC or SWC format. Please use H5 if you want to save them.")

        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                morpho.write(os.path.join(tmp_folder, "test.asc"))
                assert_equal(err.getvalue().strip(),
                             "This cell has mitochondria, they cannot be saved in  ASC or SWC format. Please use H5 if you want to save them.")

        mito = ImmutMorphology(os.path.join(tmp_folder, 'test.h5')).mitochondria
        assert_array_equal(mito.root_sections[0].diameters,
                           diameters)
        assert_array_equal(mito.root_sections[0].neurite_section_ids,
                           neuronal_section_ids)
        assert_array_equal(mito.root_sections[0].relative_path_lengths,
                           relative_pathlengths)

        assert_equal(len(mito.root_sections), 1)

        mito = Morphology(os.path.join(tmp_folder, 'test.h5')).mitochondria
        assert_equal(len(mito.root_sections), 1)
        assert_equal(mito.root_sections[0].neurite_section_ids, neuronal_section_ids)
        assert_array_equal(mito.section(0).diameters,
                           diameters)

        assert_array_equal(mito.section(0).neurite_section_ids,
                           neuronal_section_ids)

def test_duplicate_different_diameter():
    '''Test that starting a child section with a different diamete
    work as expected'''
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [1, 1, 1]]
    morpho.soma.diameters = [1, 1]

    section = morpho.append_root_section(PointLevel([[2, 2, 2], [3, 3, 3]],
                                                       [4, 4]),
                                         SectionType.axon,)

    section.append_section(PointLevel([[3, 3, 3], [4, 4, 4]], [10, 12]))
    section.append_section(PointLevel([[3, 3, 3], [5, 5, 5]], [11, 12]))


    with setup_tempdir('test_write_duplicate_different_diameter', no_cleanup=True) as tmp_folder:
        for ext in ['asc', 'h5', 'swc']:
            with captured_output() as (_, err):
                with ostream_redirect(stdout=True, stderr=True):
                    outfile = os.path.join(tmp_folder, 'tmp.' + ext)
                    morpho.write(outfile)

                    read = Morphology(outfile)

            assert_equal(len(read.root_sections[0].children), 2)
            child1, child2 = read.root_sections[0].children
            assert_array_equal(child1.points, [[3, 3, 3], [4, 4, 4]])
            assert_array_equal(child2.points, [[3, 3, 3], [5, 5, 5]])
            assert_array_equal(child1.diameters, [10, 12])
            assert_array_equal(child2.diameters, [11, 12])


def test_single_point_root_section():
    m = Morphology()
    points = []
    diameters = []

    # Too hide the warning: appending empty section
    with captured_output():
        with ostream_redirect(stdout=True, stderr=True):
            m.append_root_section(PointLevel(points, diameters), SectionType(2))

            with setup_tempdir('test_single_point_root_section', no_cleanup=True) as tmp_folder:
                assert_raises(SectionBuilderError, m.write, os.path.join(tmp_folder, "h5/empty_vasculature.h5"))

    m = Morphology()
    points = [[1., 1., 1.]]
    diameters = [2.]
    m.append_root_section(PointLevel(points, diameters), SectionType(2))

    with setup_tempdir('test_single_point_root_section', no_cleanup=True) as tmp_folder:
        assert_raises(SectionBuilderError, m.write, os.path.join(tmp_folder, "h5/empty_vasculature.h5"))
