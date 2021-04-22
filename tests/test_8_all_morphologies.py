import pytest
import subprocess

from morphio import Morphology, SomaError, RawDataError, MorphioError
from morphio.mut import Morphology as MutMorphology


bad_files = {'MorphologyRepository/Vincent Delattre/vd100714C/vd100714C_idA.ASC': SomaError,
             'MorphologyRepository/Vincent Delattre/vd100714B/vd100714B_idB.ASC': RawDataError,
             'MorphologyRepository/Shruti Muralidhar/sm100614a2/sm100614a2.ASC': RawDataError,
             'MorphologyRepository/Rodrigo Perin/rp120607_P_1-2/rp120607_P_1-2_idE.ASC': SomaError,
             'MorphologyRepository/Shruti Muralidhar/sm080904a1-6/._sm080904a1.asc': SomaError,
             'MorphologyRepository/Rodrigo Perin/rp110616_C1/rp110616_C1_idB.ASC': SomaError,

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


@pytest.mark.skip('backward compatible with nottest by nose')
def test_all():
    try:
        subprocess.call(['git', 'clone', 'ssh://bbpcode.epfl.ch/experiment/MorphologyRepository'])
    except:
        pass

    not_same_same = list()

    def walk(folder):
        import os
        for root, _, files in os.walk(folder):
            for file in files:
                yield os.path.join(root, file)

    # for i, f in enumerate(filter(_is_good_file, walk('MorphologyRepository'))):
    for i, f in enumerate(not_same_same_list):
        print(i)
        print(f)
        m = MutMorphology(f)
        immut = Morphology(f)
        try:
            m.write_h5('morph.h5')
        except MorphioError:
            continue

        m.write_swc('morph.swc')
        m.write_asc('morph.asc')
        same_same = ((immut == Morphology("morph.h5")) and
                     (immut == Morphology("morph.swc")) and
                     (immut == Morphology("morph.asc")))

        if not same_same:
            not_same_same.append(f)
    print(not_same_same)


not_same_same_list = ['MorphologyRepository/Ani Gupta/C030796A/C030796A-P2.asc',
                      'MorphologyRepository/Ani Gupta/C030397A/C030397A-P1.asc',
                      'MorphologyRepository/Ani Gupta/C220797A/C220797A-I2.asc',
                      'MorphologyRepository/Ani Gupta/C230797B/C230797B-I2.asc',
                      'MorphologyRepository/Ani Gupta/C050896A/C050896A-I.asc',
                      'MorphologyRepository/Ani Gupta/C261296A/C261296A-P2.asc',
                      'MorphologyRepository/Ani Gupta/C261296A/C261296A-P3.asc',
                      'MorphologyRepository/Ani Gupta/C040896A/C040896A-P2.asc',
                      'MorphologyRepository/Maria Toledo/mtC030800C/mtC030800C_idA.ASC']
