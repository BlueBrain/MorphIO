from itertools import product
from pathlib import Path

import numpy as np
from morphio import Morphology, RawDataError, SomaError, ostream_redirect
from numpy.testing import assert_array_almost_equal, assert_array_equal

import pytest

from utils import _test_asc_exception, tmp_asc_file

DATA_DIR = Path(__file__).parent / 'data'

NEUROLUCIDA_MARKERS = [
 'Dot',
 'Plus',
 'Cross',
 'Splat',
 'Flower',
 'Circle',
 'Flower',
 'TriStar',
 'OpenStar',
 'Asterisk',
 'SnowFlake',
 'OpenCircle',
 'ShadedStar',
 'FilledStar',
 'TexacoStar',
 'MoneyGreen',
 'DarkYellow',
 'OpenSquare',
 'OpenDiamond',
 'CircleArrow',
 'CircleCross',
 'OpenQuadStar',
 'DoubleCircle',
 'FilledSquare',
 'MalteseCross',
 'FilledCircle',
 'FilledDiamond',
 'FilledQuadStar',
 'OpenUpTriangle',
 'FilledUpTriangle',
 'OpenDownTriangle',
 'FilledDownTriangle',
 ]

def test_soma():
    with tmp_asc_file('''("CellBody"
                         (Color Red)
                         (CellBody)
                         (1 1 0 1 S1)
                         (-1 1 0 1 S2)
                         (-1 -1 0 2 S3)
                         )''') as tmp_file:

        n = Morphology(tmp_file.name)

        assert_array_equal(n.soma.points,
                           [[1, 1, 0],
                            [-1, 1, 0],
                            [-1, -1, 0]])

        assert len(n.root_sections) == 0


def test_parse_number_with_plus_symbol():
    with tmp_asc_file('''("CellBody"
                         (Color Red)
                         (CellBody)
                         (1  1 0 1 S1)
                         (1 +1 0 1 S2)  ; <- there is a '+' symbol
                         (1  1 0 2 S3)
                         )''') as tmp_file:
        n = Morphology(tmp_file.name)
        assert_array_equal(n.soma.points,
                           [[1, 1, 0],
                            [1, 1, 0],
                            [1, 1, 0]])


def test_unknown_token():
    _test_asc_exception('''
                   ("CellBody"
                   (Color Red)
                   (CellBody)
                   (1 1 0 1 S1)
                   (Z 1 0 1 S2) ; <-- Z is a BAD token
                   (-1 -1 0 2 S3)
                   )''',
                        RawDataError,
                        "Unexpected token: Z",
                        ":6:error")


def test_unfinished_point():
    _test_asc_exception('''("CellBody"
                         (Color Red)
                         (CellBody)
                         (1 1''',
                        RawDataError,
                        'Error converting: "" to float',
                        ':4:error')


def test_multiple_soma():
    _test_asc_exception('''
                             ("CellBody"
                             (Color Red)
                             (CellBody)
                             (1 1 0 1 S1)
                             (-1 1 0 1 S2)
                             )

                            ("CellBody"
                             (Color Red)
                             (CellBody)
                             (1 1 0 1 S1)
                             (-1 1 0 1 S2)
                             )''',
                        SomaError,
                        'A soma is already defined',
                        ':14:error')


def test_single_neurite_no_soma():
    with tmp_asc_file('''
                      ( (Color Yellow)
                         (Axon)
                         (Set "axons")
                         (  1.2  2.7   1.0     13)
                         (  1.2  3.7   2.0     13)
                      )''') as tmp_file:
        n = Morphology(tmp_file.name)

        assert_array_equal(n.soma.points, np.empty((0, 3)))
        assert len(n.root_sections) == 1
        assert_array_equal(n.root_sections[0].points,
                           np.array([[1.2, 2.7, 1.0],
                                     [1.2, 3.7, 2.0]], dtype=np.float32))

        assert_array_equal(n.root_sections[0].diameters,
                           np.array([13., 13.], dtype=np.float32))


def test_skip_header():
    '''Test that the header does not cause any issue'''
    with tmp_asc_file('''(FilledCircle
                         (Color RGB (64, 0, 128))
                         (Name "Marker 11")
                         (Set "axons")
                         ( -189.59    55.67    28.68     0.12)  ; 1
                         )  ;  End of markers

                         ((Color Yellow)
                         (Axon)
                         (Set "axons")
                         (  1.2  2.7   1.0     13)
                         (  1.2  3.7   2.0     13)
                         )''') as tmp_file:

        n = Morphology(tmp_file.name)
        assert len(n.root_sections) == 1
        assert_array_equal(n.root_sections[0].points,
                           np.array([[1.2, 2.7, 1.0],
                                     [1.2, 3.7, 2.0]], dtype=np.float32))


without_duplicate = '''
                     ((Dendrite)
                      (3 -4 0 5)
                      (3 -6 0 5)
                      (3 -8 0 5)
                      (3 -10 0 5)
                      (
                        (0 -10 0 2)
                        (-3 -10 0 0.3)
                        |
                        (6 -10 0 2)
                        (9 -10 0 0.3)
                      )
                      )
                     '''

with_duplicate = '''
                     ((Dendrite)
                      (3 -4 0 5)
                      (3 -6 0 5)
                      (3 -8 0 5)
                      (3 -10 0 5)
                      (
                        (3 -10 0 2) ; duplicate
                        (0 -10 0 2)
                        (-3 -10 0 0.3)
                        |
                        (3 -10 0 2) ; duplicate
                        (6 -10 0 2)
                        (9 -10 0 0.3)
                      )
                      )
                     '''

def test_read_with_duplicates():
    '''Section points are duplicated in the file
what I think the
https://developer.humanbrainproject.eu/docs/projects/morphology-documentation/0.0.2/h5v1.html
would look like'''

    with tmp_asc_file(with_duplicate) as tmp_file:
        n = Morphology(tmp_file.name)

        assert len(n.root_sections) == 1

        assert_array_equal(n.root_sections[0].points,
                           [[3, -4, 0],
                            [3, -6, 0],
                            [3, -8, 0],
                            [3, -10, 0],
                            ])

        assert_array_equal(n.root_sections[0].children[0].points,
                           [[3, -10, 0],
                            [0, -10, 0],
                            [-3, -10, 0]])
        assert_array_equal(n.root_sections[0].children[0].diameters,
                           np.array([2, 2, 0.3], dtype=np.float32))

        assert_array_equal(n.root_sections[0].children[1].points,
                           [[3, -10, 0],
                            [6, -10, 0],
                            [9, -10, 0]])
        assert_array_equal(n.root_sections[0].children[1].diameters,
                           np.array([2, 2, 0.3], dtype=np.float32))


def test_read_without_duplicates():
    with tmp_asc_file(with_duplicate) as tmp_file:
        n_with_duplicate = Morphology(tmp_file.name)

        with tmp_asc_file(without_duplicate) as tmp_file:
            n_without_duplicate = Morphology(tmp_file.name)

        assert_array_equal(n_with_duplicate.root_sections[0].children[0].points,
                           n_without_duplicate.root_sections[0].children[0].points)

        assert_array_equal(n_with_duplicate.root_sections[0].points,
                           n_without_duplicate.root_sections[0].points)

        assert_array_equal(n_with_duplicate.root_sections[0].children[0].diameters,
                           n_without_duplicate.root_sections[0].children[0].diameters)

        assert_array_equal(n_with_duplicate.root_sections[0].diameters,
                           n_without_duplicate.root_sections[0].diameters)


def test_explicit_duplicates_with_arbitrary_diameter():
    '''If the duplicate is already in the file with an arbitrary diameter, it should be preserved'''
    with tmp_asc_file('''
                     ((Dendrite)
                      (3 -4 0 5)
                      (3 -6 0 5)
                      (3 -8 0 5)
                      (3 -10 0 5)
                      (
                        (3 -10 0 20) ; duplicate
                        (0 -10 0 2)
                        (-3 -10 0 0.3)
                        |
                        (3 -10 0 2)
                        (6 -10 0 2)
                        (9 -10 0 0.3)
                      )
                      )
                     ''') as tmp_file:
        n = Morphology(tmp_file.name)
        assert_array_equal(n.root_sections[0].children[0].diameters,
                           np.array([20, 2, 0.3], dtype=np.float32))


def test_unfinished_file():
    _test_asc_exception('''
                     ((Dendrite)
                      (3 -4 0 2)
                      (3 -6 0 2)
                      (3 -8 0 2)
                      (3 -10 0 2)
                      (
                        (3 -10 0 2)
                        (0 -10 0 2)
                        (-3 -10 0 2)
                        |
                     ''',
                        RawDataError,
                        "Hit end of file while consuming a neurite",
                        ":12:error")

def test_section_single_point():
    '''Test single point section.
    The parent section last point will be duplicated
    '''
    with tmp_asc_file('''
                     ((Dendrite)
                      (3 -4 0 2)
                      (3 -6 0 2)
                      (3 -8 0 2)
                      (3 -10 0 2)
                      (
                        (3 -10 2 4)  ; single point section without duplicate
                       |
                        (3 -10 0 2)  ; normal section with duplicate
                        (3 -10 1 2)
                        (3 -10 2 2)
                      )
                     )''') as tmp_file:

        n = Morphology(tmp_file.name)
        assert len(n.root_sections) == 1
        assert len(n.root_sections[0].children) == 2
        assert_array_equal(n.root_sections[0].children[0].points,
                           np.array([[3, -10, 0],
                                     [3, -10, 2]], dtype=np.float32))
        assert_array_equal(n.root_sections[0].children[0].diameters,
                           np.array([4, 4], dtype=np.float32))

        assert_array_equal(n.root_sections[0].children[1].points,
                           np.array([[3, -10, 0],
                                     [3, -10, 1],
                                     [3, -10, 2]], dtype=np.float32))


def test_single_children():
    '''Single children are no longer (starting at v2.8) merged with their parent

    They used to be until the decision in:
    https://github.com/BlueBrain/MorphIO/issues/235
    '''
    with tmp_asc_file('''
                     ((Dendrite)
                      (3 -4 0 2)
                      (3 -6 0 2)
                      (3 -8 0 2)
                      (3 -10 0 2)
                      (
                        (3 -10 0 4)
                        (0 -10 0 4)
                        (-3 -10 0 3)
                        (
                          (-5 -5 5 5)
                          |
                          (-6 -6 6 6)
                        )
                       )
                      )
                 ''') as tmp_file:

        n = Morphology(tmp_file.name)

        assert len(n.soma.points) == 0
        assert len(n.soma.points) == 0
        assert len(n.root_sections) == 1
        assert_array_equal(n.root_sections[0].points,
                           np.array([[3, -4, 0],
                                     [3, -6, 0],
                                     [3, -8, 0],
                                     [3, -10, 0]],
                                    dtype=np.float32))
        assert len(n.root_sections[0].children) == 1


def test_spine():
    neuron = Morphology(DATA_DIR / 'spine.asc')
    assert len(neuron.root_sections) == 1
    assert_array_equal(neuron.root_sections[0].points,
                       np.array([[3.22,    -1.15,   150.00],
                                 [5.84,    -2.17,   150.00],
                                 [9.34,    -3.81,   150.00],
                                 [9.99,    -4.00,   150.00],
                                 [11.38,    -4.62,   150.00],
                                 [12.55,    -5.16,   150.00],
                                 [13.75,    -5.96,   150.00]], dtype=np.float32))


def test_single_point_section_duplicate_parent():
    '''Section is made only of the duplicate point'''
    with tmp_asc_file('''
                     ((Dendrite)
                      (3 -4 0 2)
                      (3 -10 0 2)
                      (
                        (3 -10 0 4)  ; duplicate point
                      )
                     )''') as tmp_file:

        neuron = Morphology(tmp_file.name)
        assert_array_equal(neuron.root_sections[0].points, [[  3.,  -4.,   0.],
                                                            [  3., -10.,   0.]])
        assert_array_equal(neuron.root_sections[0].diameters, np.array([2, 2], dtype=np.float32))


def test_single_point_section_duplicate_parent_complex():
    '''The following neuron represents a malformed tri-furcation. It is (badly) represented
    as a bifurcation of a bifurcation

    This is a simplification of the problem at:
    MorphologyRepository/Ani_Gupta/C030796A/C030796A-P2.asc:5915
    '''
    with tmp_asc_file('''
((Color Blue)
 (Axon)
    (1 0 0 1)
    (2 0 0 1)
    (
      (2 0 0 1) ; Bifurcation starts here
      (
        (4 0 0 1)
        |
        (5 0 0 1)
      )
      |           ; Bifurcation of the bifurcation
      (6 0 0 1)
  )
 )
''') as bad_tmp_file:
        neuron = Morphology(bad_tmp_file.name)

    children = neuron.root_sections[0].children
    assert len(children) == 3
    assert_array_equal(children[0].points, [[2, 0, 0], [4, 0, 0]])
    assert_array_equal(children[1].points, [[2, 0, 0], [5, 0, 0]])
    assert_array_equal(children[2].points, [[2, 0, 0], [6, 0, 0]])


def test_spine():
    neuron = Morphology(DATA_DIR / 'spine.asc')
    assert len(neuron.root_sections) == 1
    assert_array_equal(neuron.root_sections[0].points,
                       np.array([[3.22,    -1.15,   150.00],
                                 [5.84,    -2.17,   150.00],
                                 [9.34,    -3.81,   150.00],
                                 [9.99,    -4.00,   150.00],
                                 [11.38,    -4.62,   150.00],
                                 [12.55,    -5.16,   150.00],
                                 [13.75,    -5.96,   150.00]], dtype=np.float32))


def test_markers():
    '''Test that markers do not prevent file from being read correctly'''

    n = Morphology(DATA_DIR / 'markers.asc')

    assert len(n.markers) == 5
    assert_array_equal(n.markers[0].points,
                       np.array([[-271.87, -121.14, -16.27],
                                 [-269.34, -122.29, -15.48]],
                                dtype=np.float32))
    assert_array_equal(n.markers[0].diameters,
                       np.array([0.69, 0.69], dtype=np.float32))
    assert n.markers[0].label == 'Cross'

    assert_array_equal(n.markers[1].points,
                       np.array([[-279.41, -119.99, -18.00],
                                 [-272.98, -126.60, -21.22]],
                                dtype=np.float32))
    assert_array_equal(n.markers[1].diameters,
                       np.array([0.46, 0.92], dtype=np.float32))
    assert n.markers[1].label == 'Cross'

    assert_array_equal(n.markers[2].points,
                       np.array([[-223.67, -157.92, -42.45],
                                 [-222.76, -154.18, -39.90]],
                                dtype=np.float32))
    assert_array_equal(n.markers[2].diameters,
                       np.array([0.69, 0.69], dtype=np.float32))
    assert n.markers[2].label == 'Cross'
    assert n.markers[3].label == 'INCOMPLETE'
    assert n.markers[3].section_id == 3
    assert n.markers[4].label == 'INCOMPLETE'
    assert n.markers[4].section_id == 4


    assert len(n.root_sections) == 1

    assert_array_equal(n.root_sections[0].points,
                       np.array([[-290.87,  -113.09,   -16.32],
                                 [-290.87,  -113.09,   -16.32],
                                 ],
                                dtype=np.float32))

    assert_array_equal(n.root_sections[0].children[0].points,
                       np.array([[-290.87,  -113.09,   -16.32],
                                 [-277.14,  -119.13,   -18.02],
                                 [-275.54,  -119.99,   -16.67]],
                                dtype=np.float32))

    assert_array_equal(n.root_sections[0].children[1].points,
                       np.array([[-290.87,  -113.09,   -16.32],
                                 [-277.80,  -120.28,   -19.48],
                                 [-276.65,  -121.14,   -20.20]],
                                dtype=np.float32))

    assert_array_equal(n.root_sections[0].children[1].children[0].points,
                       np.array([[-276.65,  -121.14,   -20.20],
                                 [-267.94,  -128.61,   -22.57],
                                 [-204.90,  -157.63,   -42.45]],
                                dtype=np.float32))

    assert_array_equal(n.root_sections[0].children[1].children[1].points,
                       np.array([[-276.65,  -121.14,   -20.20],
                                 [-269.77,  -129.47,   -22.57],
                                 [-268.17,  -130.62,   -24.75],
                                 [-266.79,  -131.77,   -26.13]],
                                dtype=np.float32))


def test_string_markers():
    cell = Morphology(DATA_DIR / 'pia.asc')

    # The for loop tests that the various constructors keep the markers alive
    for m in (cell, cell.as_mutable(), cell.as_mutable().as_immutable()):
        assert len(m.root_sections) == 1
        assert_array_equal(m.root_sections[0].points,
                           np.array([[-2.87, -9.24, -5.06],
                                     [-2.76, -10.41, -5.13],
                                     [-2.03, -12.48, -5.13],
                                     [-1.62, -13.30, -5.56]], dtype=np.float32))

        assert len(m.markers) == 2
        pia = m.markers[0]
        assert pia.label == 'pia'
        assert_array_equal(pia.points,
                           [[0, 1, 2],
                            [3, 4, 5],
                            [6, 7, 8],
                            [9, 10, 11]])
        assert_array_equal(pia.diameters, [3, 4, 5, 6])

        assert m.markers[1].label == 'layer1-2'
        assert_array_equal(m.markers[1].points,
                           np.array([[983.07, 455.36, -0.19],
                                     [1192.31, 420.35, -0.19]], dtype=np.float32))
        assert_array_equal(m.markers[1].diameters, np.array([0.15, 0.15], dtype=np.float32))

def test_neurolucida_markers():
    SIMPLE = Morphology(DATA_DIR / 'simple.asc')
    for marker, suffix in product(NEUROLUCIDA_MARKERS, ['', '7', '123']):
        marker += suffix
        with tmp_asc_file(f'''
({marker}
  (Color White)
  (Name "fat end")
  (   81.58   -77.98   -20.32     0.50)  ; 1
)  ;  End of markers

("CellBody"
 (Color Red)
 (CellBody)
 (0 0 0 2)
 )


({marker}
  (Color White)
  (Name "fat end")
  (   51.58   -77.78   -24.32     0.52)  ; 1
)  ;  End of markers

 ((Dendrite)
  (0 0 0 2)
  (0 5 0 2)
  (
   (-5 5 0 3)
   |
   (6 5 0 3)
   )
  )


 ((Axon)
  (0 0 0 2)
  (0 -4 0 2)
  (
   (6 -4 0 4)
   |
   (-5 -4 0 4)
   )
  )
                         )''') as tmp_file:
            neuron = Morphology(tmp_file.name)

            assert_array_equal(neuron.points, SIMPLE.points)
            assert len(neuron.markers) == 2
            assert_array_almost_equal(neuron.markers[0].points,
                                      np.array([[81.58, -77.98, -20.32]], dtype=np.float32))
            assert_array_almost_equal(neuron.markers[0].diameters,
                                      np.array([0.5], dtype=np.float32))
            assert_array_almost_equal(neuron.markers[1].points,
                                      np.array([[51.580002, -77.779999, -24.32]],
                                               dtype=np.float32))
            assert_array_almost_equal(neuron.markers[1].diameters,
                                      np.array([0.52], dtype=np.float32))


def test_invalid_incomplete():
    '''Test that any data after Incomplete is invalid.'''
    with pytest.raises(RawDataError, match='Incomplete'):
        Morphology(DATA_DIR / 'invalid-incomplete.asc')


def test_skip_font():
    assert_array_equal(Morphology(DATA_DIR / 'simple-with-font.asc').points,
                       Morphology(DATA_DIR / 'simple.asc').points)

def test_skip_imagecoord():
    assert_array_equal(Morphology(DATA_DIR / 'simple-with-image-coord.asc').points,
                       Morphology(DATA_DIR / 'simple.asc').points)

def test_Sections_block():
    assert_array_equal(Morphology(DATA_DIR / 'sections-block.asc').points,
                       Morphology(DATA_DIR / 'simple.asc').points)

def test_marker_with_string():
    m = Morphology(DATA_DIR / 'marker-with-string.asc')
    assert_array_equal(m.markers[0].points, np.array([[  -0.97    , -141.169998,   84.769997]],
                                                     dtype=np.float32))
def test_version():
    assert_array_equal(Morphology(DATA_DIR / 'simple.asc').version, ('asc', 1, 0))
