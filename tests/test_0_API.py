from nose.tools import ok_, assert_equal
import morphio


def test_doc_exists():
    cls = morphio.Morphology

    classes = [morphio.Morphology,
               morphio.mut.Morphology,
               morphio.Section,
               morphio.mut.Section,
               morphio.Soma,
               morphio.mut.Soma,
               morphio.MitoSection,
               morphio.mut.MitoSection,
               morphio.Mitochondria,
               morphio.mut.Mitochondria
               ]
    for cls in classes:
        public_methods = (method for method in dir(cls) if not method[:2] == '__')
        for method in public_methods:
            ok_(getattr(cls, method).__doc__,
                'Public method {} of class {} is not documented !'.format(method, cls))

def test_mut_immut_have_same_methods():
    '''Both modules morphio and morphio.mut should expose the same API'''
    def methods(cls):
        return set(method for method in dir(cls) if not method[:2] == '__')

    only_in_immut = {'section_types', 'diameters', 'perimeters', 'points', 'as_mutable'}
    only_in_mut = {'write', 'append_root_section', 'delete_section', 'build_read_only', 'as_immutable'}
    assert_equal(methods(morphio.Morphology) - only_in_immut,
                 methods(morphio.mut.Morphology) - only_in_mut)

    assert_equal(methods(morphio.Section),
                 methods(morphio.mut.Section) - {'append_section'})

    assert_equal(methods(morphio.Soma),
                 methods(morphio.mut.Soma))

    # assert_equal(methods(morphio.Mitochondria),
    #              methods(morphio.mut.Mitochondria))
