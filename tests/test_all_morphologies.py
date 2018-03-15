from nose.tools import ok_, nottest
import subprocess

from morphio import Morphology, SomaError, RawDataError
from morphio.mut import Morphology as MutMorphology


bad_files = {'MorphologyRepository/Vincent Delattre/vd100714C/vd100714C_idA.ASC': SomaError,
             'MorphologyRepository/Vincent Delattre/vd100714B/vd100714B_idB.ASC': RawDataError,
             'MorphologyRepository/Shruti Muralidhar/sm100614a2/sm100614a2.ASC': RawDataError,
             'MorphologyRepository/Rodrigo Perin/rp120607_P_1-2/rp120607_P_1-2_idE.ASC': SomaError,
             'MorphologyRepository/Thomas Berger/tkb070202a2/tkb070202a2_ch0_cc2_n_tb_100x_3.asc': RawDataError,
             'MorphologyRepository/Thomas Berger/tkb061122a2/tkb061122a2_ch1_ct_n_rm_100x_1.asc': RawDataError,
             'MorphologyRepository/Thomas Berger/tkb051201b1/tkb051201b1_ch7_ct_n_es_100x_1.asc': RawDataError,

             'MorphologyRepository/Thomas Berger/tkb060123a2/tkb060123a2_ch2_ct_x_db_60x_2.asc': RawDataError,
             'MorphologyRepository/Cristina Radaelli/cr161021_A/cr161021_A_idB.ASC': RawDataError,
             'MorphologyRepository/Cristina Radaelli/cr161027_A/cr161027_A_idA.ASC': SomaError,
             'MorphologyRepository/Olivier Gschwend/og060829a1-4/og060829a1-4_idB.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C271097A/C271097A-I4.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C030397A/C030397A-P3.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C310897B/C310897B-P3.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C250298A/C250298A-I4.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C180298A/C180298A-I4.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C261296A/C261296A-P1.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C260897C/C260897C-P4.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C220197A/C220197A-P3.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C200897C/C200897C-I1.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C040896A/C040896A-P3.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C120398A/C120398A-I4.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C310897A/C310897A-I2.asc': RawDataError,
             'MorphologyRepository/Ani Gupta/C240298B/C240298B-I4.asc': RawDataError,
             'MorphologyRepository/Brandi Mattson/C280206C/C280206C.asc': RawDataError,
             'MorphologyRepository/Brandi Mattson/C280206C/C280206C - Copy.asc': RawDataError,
             'MorphologyRepository/Brandi Mattson/C270106D/C270106D.asc': RawDataError,

             'MorphologyRepository/Ani Gupta/C050896A/C050896A-P2.asc': RawDataError,  # 'Unexpected token: |'
             }


def _is_good_file(filename):
    return filename not in bad_files.keys() and filename.lower().endswith('.asc')


@nottest
def test_all():
    try:
        subprocess.call(['git', 'clone', 'ssh://bbpcode.epfl.ch/experiment/MorphologyRepository'])
    except:
        pass

    def walk(folder):
        import os
        for root, _, files in os.walk(folder):
            for file in files:
                yield os.path.join(root, file)

    for f in filter(_is_good_file, walk('MorphologyRepository')):
        print(f)
        m = MutMorphology(f)
        m.write_h5('test.h5')
        m.write_swc('test.swc')
        ok_(Morphology(f) == Morphology('test.swc'))
        # m.write_asc('test.asc')
