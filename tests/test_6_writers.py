import h5py
import numpy as np
import pytest
from numpy.testing import assert_array_equal
from utils import captured_output

from morphio import MitochondriaPointLevel
from morphio import Morphology as ImmutMorphology
from morphio import (PointLevel, SectionBuilderError, SectionType, WriterError,
                     ostream_redirect, SomaType)
from morphio.mut import Morphology


def test_write_empty_file(tmp_path):
    '''Check that empty morphology are not written to disk'''
    with captured_output() as (_, _):
        with ostream_redirect(stdout=True, stderr=True):
            for ext in ['asc', 'swc', 'h5']:
                outname = tmp_path / f'empty.{ext}'
                Morphology().write(outname)
                assert not outname.exists()


def test_write_soma_basic(tmp_path):
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [0, 0, 0], [0, 0, 0]]
    morpho.soma.diameters = [2, 3, 3]

    morpho.write(tmp_path / "test_write.swc")
    morpho.write(tmp_path / "test_write_pathlib.swc")
    assert len(list(tmp_path.glob('*.*'))) == 2


def test_write_basic(tmp_path):
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

    morpho.write(tmp_path / "test_write.asc")
    morpho.write(tmp_path / "test_write.swc")
    h5_out = tmp_path / "test_write.h5"
    morpho.write(h5_out)

    expected = [[0., 0., 0.], [0., 5., 0.], [0., 5., 0.], [-5., 5., 0.],
                [0., 5., 0.], [6., 5., 0.], [0., 0., 0.], [0., -4., 0.],
                [0., -4., 0.], [6., -4., 0.], [0., -4., 0.], [-5., -4., 0.]]
    assert_array_equal(ImmutMorphology(tmp_path / "test_write.asc").points, expected)
    assert_array_equal(ImmutMorphology(tmp_path / "test_write.swc").points, expected)
    h5_morph = ImmutMorphology(tmp_path / "test_write.h5")
    assert_array_equal(h5_morph.points, expected)
    assert h5_morph.version == ('h5', 1, 3)

    with h5py.File(h5_out, 'r') as h5_file:
        assert '/perimeters' not in h5_file.keys()


def test_write_merge_only_child_asc_h5(tmp_path):
    '''The root section has only one child

    When writing, children should *not* be merged with their parent section.

    Note: See `test_write_merge_only_child_swc` for the SWC case.
    '''

    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    root = morpho.append_root_section(PointLevel([[0, 0, 0],
                                                  [0, 5, 0]],
                                                 [2, 2]),
                                      SectionType.basal_dendrite)
    root.append_section(PointLevel([[0, 5, 0], [0, 6, 0]], [2, 3]))

    for extension in ['asc', 'h5']:
        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                filename = tmp_path / 'test.{}'.format(extension)
                morpho.write(filename)

        read = Morphology(filename)

        root = read.root_sections[0]
        assert_array_equal(root.points,
                           [[0, 0, 0],
                            [0, 5, 0]])
        assert len(root.children) == 1


def test_write_merge_only_child_swc():
    '''Attempts to write a morphology with unifurcations with SWC should result in an exception.'''
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    root = morpho.append_root_section(
                                 PointLevel([[0, 0, 0],
                                             [0, 5, 0]],
                                            [2, 2]),
                                 SectionType.basal_dendrite)
    root.append_section(PointLevel([[0, 5, 0], [0, 6, 0]], [2, 3]))

    with pytest.raises(WriterError,
                       match=('Section 0 has a single child section. '
                              'Single child section are not allowed when writing to SWC format. '
                              'Please sanitize the morphology first.')):
       morpho.write('/tmp/bla.swc')  # the path does not need to exists since it will fail before


def test_write_perimeter(tmp_path):
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

    h5_out = tmp_path / "test_write.h5"
    morpho.write(h5_out)

    assert_array_equal(ImmutMorphology(h5_out).perimeters,
                       [5., 6., 6., 7., 6., 8.])

    # Cannot right a morph with perimeter data to ASC and SWC
    for ext in ['swc', 'asc']:
        with pytest.raises(WriterError):
            morpho.write(tmp_path / f"test_write_perimeter.{ext}")


def test_write_no_soma(tmp_path):
    morpho = Morphology()
    morpho.append_root_section(PointLevel([[0, 0, 0],
                                           [0, 5, 0]],
                                          [2, 2]),
                               SectionType.basal_dendrite)
    morpho.append_root_section(PointLevel([[0, 1, 0],
                                           [0, 7, 0]],
                                          [2, 2]),
                               SectionType.basal_dendrite)

    for ext, soma_type in (('asc', SomaType.SOMA_SIMPLE_CONTOUR),
                           ('h5', SomaType.SOMA_SIMPLE_CONTOUR),
                           ('swc', SomaType.SOMA_CYLINDERS)):
        morpho.soma.type = soma_type
        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                outfile = tmp_path / f'tmp.{ext}'
                morpho.write(outfile)
                assert (err.getvalue().strip() ==
                             'Warning: writing file without a soma')
                read = Morphology(outfile)

        assert len(read.soma.points) == 0
        assert len(read.root_sections) == 2
        assert_array_equal(read.root_sections[0].points, [[0, 0, 0], [0, 5, 0]])
        assert_array_equal(read.root_sections[1].points, [[0, 1, 0], [0, 7, 0]])


def test_write_soma__points_no_diameters(tmp_path):

    morph = Morphology()
    morph.soma.points = [[0., 0., 0.]]

    for ext in ["asc", "h5", "swc"]:
        with pytest.raises(WriterError):
            morph.write(tmp_path / f"tmp.{ext}")


def test_mitochondria(tmp_path):
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0], [1, 1, 1], [0, 0, 0]]
    morpho.soma.diameters = [1, 1, 1]
    morpho.soma.type = SomaType.SOMA_SIMPLE_CONTOUR

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
    morpho.write(tmp_path / "test.h5")

    mito = ImmutMorphology(tmp_path / 'test.h5').mitochondria
    assert_array_equal(mito.root_sections[0].diameters,
                       diameters)
    assert_array_equal(mito.root_sections[0].neurite_section_ids,
                       neuronal_section_ids)
    assert_array_equal(mito.root_sections[0].relative_path_lengths,
                       relative_pathlengths)
    assert len(mito.root_sections) == 1

    mito = Morphology(tmp_path / 'test.h5').mitochondria
    assert len(mito.root_sections) == 1
    assert mito.root_sections[0].neurite_section_ids == neuronal_section_ids
    assert_array_equal(mito.section(0).diameters,
                       diameters)
    assert_array_equal(mito.section(0).neurite_section_ids,
                       neuronal_section_ids)

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morpho.write(tmp_path / "test.asc")
            assert err.getvalue().strip() == (
                "Warning: this cell has mitochondria, they cannot be saved in "
                " ASC or SWC format. Please use H5 if you want to save them.")

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morpho.soma.type = SomaType.SOMA_CYLINDERS
            morpho.write(tmp_path / "test.swc")
            assert err.getvalue().strip() == (
                "Warning: this cell has mitochondria, they cannot be saved in "
                " ASC or SWC format. Please use H5 if you want to save them.")



def test_duplicate_different_diameter(tmp_path):
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

    for ext in ['asc', 'h5', 'swc']:
        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                outfile = tmp_path / f'tmp.{ext}'
                morpho.write(outfile)

                read = Morphology(outfile)

        assert len(read.root_sections[0].children) == 2
        child1, child2 = read.root_sections[0].children
        assert_array_equal(child1.points, [[3, 3, 3], [4, 4, 4]])
        assert_array_equal(child2.points, [[3, 3, 3], [5, 5, 5]])
        assert_array_equal(child1.diameters, [10, 12])
        assert_array_equal(child2.diameters, [11, 12])


def test_single_point_root_section(tmp_path):
    m = Morphology()
    points = []
    diameters = []

    # To hide warning: appending empty section
    with captured_output():
        with ostream_redirect(stdout=True, stderr=True):
            m.append_root_section(PointLevel(points, diameters), SectionType(2))
            with pytest.raises(SectionBuilderError):
                m.write(tmp_path / "h5/empty_vasculature.h5")

    m = Morphology()
    points = [[1., 1., 1.]]
    diameters = [2.]
    m.append_root_section(PointLevel(points, diameters), SectionType(2))

    with pytest.raises(SectionBuilderError):
        m.write(tmp_path / "h5/empty_vasculature.h5")


def test_write_custom_property__throws(tmp_path):
    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    morpho.append_root_section(
        PointLevel([[0, 0, 0], [0, 5, 0]], [2, 2]),
        SectionType.custom5
    )

    with pytest.raises(WriterError):
        morpho.write(tmp_path / "test_write.asc")

    morpho = Morphology()
    morpho.soma.points = [[0, 0, 0]]
    morpho.soma.diameters = [2]

    morpho.append_root_section(
        PointLevel([[0, 0, 0], [0, 5, 0]], [2, 2]),
        SectionType.custom5
    )

    dendrite = morpho.append_root_section(PointLevel([[0, 0, 0], [0, 5, 0]], [2, 2]),
                                          SectionType.basal_dendrite)

    dendrite.append_section(PointLevel([[0, 5, 0], [-5, 5, 0]], [2, 3]), SectionType.custom5)

    with pytest.raises(WriterError):
        morpho.write(tmp_path / "test_write.asc")


def test_write_soma_types(tmp_path):
    morph = Morphology()
    morph.soma.points = [[0, 0, 0]]
    morph.soma.diameters = [2]
    # by default, soma start off as SomaType.SOMA_UNDEFINED

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morph.write(tmp_path / "SOMA_UNDEFINED.asc")
    assert err.getvalue().strip() == (
        'Soma must be a contour for ASC and H5: '
        'see https://github.com/BlueBrain/MorphIO/issues/457')

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morph.write(tmp_path / "SOMA_UNDEFINED.h5")
    assert err.getvalue().strip() == (
        'Soma must be a contour for ASC and H5: '
        'see https://github.com/BlueBrain/MorphIO/issues/457')

    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            morph.write(tmp_path / "SOMA_UNDEFINED.swc")
    assert err.getvalue().strip() == (
        'Soma must be stacked cylinders or a point: '
        'see https://github.com/BlueBrain/MorphIO/issues/457')

def test_write_soma_invariants(tmp_path):
    morph = Morphology()
    morph.soma.points = [[0, 0, 0]]
    morph.soma.diameters = [2]
    morph.soma.type = SomaType.SOMA_SIMPLE_CONTOUR

    with pytest.raises(WriterError):
        morph.write(tmp_path / "test_write.asc")

    with pytest.raises(WriterError):
        morph.write(tmp_path / "test_write.h5")
