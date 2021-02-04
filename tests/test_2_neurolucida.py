import os
from pathlib import Path

import morphio
import numpy as np
from morphio import Morphology, RawDataError, SomaError, ostream_redirect
from nose import tools as nt
from nose.tools import assert_equal, eq_, ok_
from numpy.testing import assert_array_almost_equal, assert_array_equal

from .utils import _test_asc_exception, assert_substring, captured_output, tmp_asc_file

DATA_DIR = Path(__file__).parent / 'data'

NEUROLUCIDA_MARKERS = ['Dot', 'FilledCircle', 'SnowFlake', 'Asterisk', 'OpenCircle',
                       'OpenStar', 'Flower', 'OpenSquare', 'FilledStar', 'DoubleCircle', 'Circle1']


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

        nt.assert_equal(len(n.root_sections), 0)


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
        nt.assert_equal(len(n.root_sections), 1)
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
        nt.assert_equal(len(n.root_sections), 1)
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

    nt.assert_equal(len(n.root_sections), 1)

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


def test_empty_sibling():
    '''The empty sibling will be removed and the single child will be merged
    with its parent'''
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            with tmp_asc_file('''((Dendrite)
                      (3 -4 0 10)
                      (3 -6 0 9)
                      (3 -8 0 8)
                      (3 -10 0 7)
                      (
                        (3 -10 0 6)
                        (0 -10 0 5)
                        (-3 -10 0 4)
                        |       ; <-- empty sibling but still works !
                       )
                      )
                 ''') as tmp_file:
                n = Morphology(tmp_file.name)
                assert_substring('is the only child of section: 0 starting at:',
                                 err.getvalue().strip())
                assert_substring('It will be merged with the parent section',
                                 err.getvalue().strip())

    assert_equal(len(n.root_sections), 1)
    assert_array_equal(n.root_sections[0].points,
                       np.array([[3, -4, 0],
                                 [3, -6, 0],
                                 [3, -8, 0],
                                 [3, -10, 0],
                                 [0, -10, 0],
                                 [-3, -10, 0]],
                                dtype=np.float32))
    assert_array_equal(n.root_sections[0].diameters,
                       np.array([10, 9, 8, 7, 5, 4], dtype=np.float32))

    assert_equal(len(n.annotations), 1)
    annotation = n.annotations[0]
    assert_equal(annotation.type, morphio.AnnotationType.single_child)
    assert_equal(annotation.line_number, 6)
    assert_array_equal(annotation.points, [[3, -10, 0], [0, -10, 0], [-3, -10, 0]])
    assert_array_equal(annotation.diameters, [6, 5, 4])

def test_nested_single_child():
    with captured_output() as (_, err):
        with ostream_redirect(stdout=True, stderr=True):
            n = Morphology(DATA_DIR / 'nested_single_children.asc')
    assert_array_equal(n.root_sections[0].points,
                       [[0., 0., 0.],
                        [0., 0., 1.],
                        [0., 0., 2.],
                        [0., 0., 3.],
                        [0., 0., 4.]])
    assert_array_equal(n.root_sections[0].diameters, np.array([8, 7, 6, 5, 4], dtype=np.float32))


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
        nt.assert_equal(len(n.root_sections), 1)
        nt.assert_equal(len(n.root_sections[0].children), 2)
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
    '''Single children are merged with their parent'''
    with tmp_asc_file('''
                     ((Dendrite)
                      (3 -4 0 2)
                      (3 -6 0 2)
                      (3 -8 0 2)
                      (3 -10 0 2)
                      (
                        (3 -10 0 4)  ; merged with parent section
                        (0 -10 0 4)  ; merged with parent section
                        (-3 -10 0 3) ; merged with parent section
                        (
                          (-5 -5 5 5)
                          |
                          (-6 -6 6 6)
                        )
                       )
                      )
                 ''') as tmp_file:

        with captured_output() as (_, err):
            with ostream_redirect(stdout=True, stderr=True):
                n = Morphology(tmp_file.name)
                assert_substring('is the only child of section: 0 starting at:',
                                 err.getvalue().strip())
                assert_substring('It will be merged with the parent section',
                                 err.getvalue().strip())

        nt.assert_equal(len(n.soma.points), 0)
        nt.assert_equal(len(n.soma.points), 0)
        assert_equal(len(n.root_sections), 1)
        assert_array_equal(n.root_sections[0].points,
                           np.array([[3, -4, 0],
                                     [3, -6, 0],
                                     [3, -8, 0],
                                     [3, -10, 0],
                                     [0, -10, 0],
                                     [-3, -10, 0]],
                                    dtype=np.float32))
        assert_array_equal(n.root_sections[0].diameters,
                           np.array([2, 2, 2, 2, 4, 3], dtype=np.float32))

        assert_equal(len(n.root_sections[0].children), 2)

        assert_array_equal(n.root_sections[0].children[0].points,
                           np.array([[-3, -10, 0],
                                     [-5, -5, 5]]))

        assert_array_equal(n.root_sections[0].children[1].points,
                           np.array([[-3, -10, 0],
                                     [-6, -6, 6]]))


def test_spine():
    neuron = Morphology(DATA_DIR / 'spine.asc')
    assert_equal(len(neuron.root_sections), 1)
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
    assert_equal(len(children), 3)
    assert_array_equal(children[0].points, [[2, 0, 0], [4, 0, 0]])
    assert_array_equal(children[1].points, [[2, 0, 0], [5, 0, 0]])
    assert_array_equal(children[2].points, [[2, 0, 0], [6, 0, 0]])


def test_spine():
    neuron = Morphology(DATA_DIR / 'spine.asc')
    assert_equal(len(neuron.root_sections), 1)
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

    with tmp_asc_file('''
( (Color White)  ; [10,1]
  (Dendrite)
  ( -290.87  -113.09   -16.32     2.06)  ; Root
  ( -290.87  -113.09   -16.32     2.06)  ; R, 1
  (
    ( -277.14  -119.13   -18.02     0.69)  ; R-1, 1
    ( -275.54  -119.99   -16.67     0.69)  ; R-1, 2
    (Cross  ;  [3,3]
      (Color Orange)
      (Name "Marker 3")
      ( -271.87  -121.14   -16.27     0.69)  ; 1
      ( -269.34  -122.29   -15.48     0.69)  ; 2
    )  ;  End of markers
     Normal
  |
    ( -277.80  -120.28   -19.48     0.92)  ; R-2, 1
    ( -276.65  -121.14   -20.20     0.92)  ; R-2, 2
    (Cross  ;  [3,3]
      (Color Orange)
      (Name "Marker 3")
      ( -279.41  -119.99   -18.00     0.46)  ; 1
      ( -272.98  -126.60   -21.22     0.92)  ; 2
    )  ;  End of markers
    (
      ( -267.94  -128.61   -22.57     0.69)  ; R-2-1, 1
      ( -204.90  -157.63   -42.45     0.69)  ; R-2-1, 34
      (Cross  ;  [3,3]
        (Color Orange)
        (Name "Marker 3")
        ( -223.67  -157.92   -42.45     0.69)  ; 1
        ( -222.76  -154.18   -39.90     0.69)  ; 2
      )  ;  End of markers
       Incomplete
    |
      ( -269.77  -129.47   -22.57     0.92)  ; R-2-2, 1
      ( -268.17  -130.62   -24.75     0.92)  ; R-2-2, 2
      ( -266.79  -131.77   -26.13     0.92)  ; R-2-2, 3
       Incomplete
    )  ;  End of split
  )  ;  End of split
)
''') as tmp_file:

        n = Morphology(tmp_file.name)

        nt.assert_equal(len(n.root_sections), 1)

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
        assert_equal(len(m.root_sections), 1)
        assert_array_equal(m.root_sections[0].points,
                           np.array([[-2.87, -9.24, -5.06],
                                     [-2.76, -10.41, -5.13],
                                     [-2.03, -12.48, -5.13],
                                     [-1.62, -13.30, -5.56]], dtype=np.float32))

        assert_equal(len(m.markers), 2)
        pia = m.markers[0]
        assert_equal(pia.label, 'pia')
        assert_array_equal(pia.points,
                           [[0, 1, 2],
                            [3, 4, 5],
                            [6, 7, 8],
                            [9, 10, 11]])
        assert_array_equal(pia.diameters, [3, 4, 5, 6])

        assert_equal(m.markers[1].label, 'layer1-2')
        assert_array_equal(m.markers[1].points,
                           np.array([[983.07, 455.36, -0.19],
                                     [1192.31, 420.35, -0.19]], dtype=np.float32))
        assert_array_equal(m.markers[1].diameters, np.array([0.15, 0.15], dtype=np.float32))

def test_neurolucida_markers():
    SIMPLE = Morphology(DATA_DIR / 'simple.asc')
    for marker in NEUROLUCIDA_MARKERS[:1]:
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
        assert_equal(len(neuron.markers), 2)
        assert_array_almost_equal(neuron.markers[0].points,
                                  np.array([[81.58, -77.98, -20.32]], dtype=np.float32))
        assert_array_almost_equal(neuron.markers[0].diameters,
                                  np.array([0.5], dtype=np.float32))
        assert_array_almost_equal(neuron.markers[1].points,
                                  np.array([[51.580002, -77.779999, -24.32]],
                                           dtype=np.float32))
        assert_array_almost_equal(neuron.markers[1].diameters,
                                  np.array([0.52], dtype=np.float32))

def test_skip_font():
    assert_array_equal(Morphology(DATA_DIR / 'simple-with-font.asc').points,
                       Morphology(DATA_DIR / 'simple.asc').points)

def test_skip_imagecoord():
    assert_array_equal(Morphology(DATA_DIR / 'simple-with-image-coord.asc').points,
                       Morphology(DATA_DIR / 'simple.asc').points)

def test_Sections_block():
    assert_array_equal(Morphology(DATA_DIR / 'sections-block.asc').points,
                       Morphology(DATA_DIR / 'simple.asc').points)
