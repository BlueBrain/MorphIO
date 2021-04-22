import morphio


def test_doc_exists():
    cls = morphio.Morphology

    classes = [
        morphio.EndoplasmicReticulum,
        morphio.MitoSection,
        morphio.Mitochondria,
        morphio.Morphology,
        morphio.Section,
        morphio.Soma,
        morphio.mut.EndoplasmicReticulum,
        morphio.mut.MitoSection,
        morphio.mut.Mitochondria,
        morphio.mut.Morphology,
        morphio.mut.Section,
        morphio.mut.Soma,
    ]
    for cls in classes:
        public_methods = (method for method in dir(cls) if not method[:2] == '__')
        for method in public_methods:
            assert getattr(cls, method).__doc__, \
                'Public method {} of class {} is not documented !'.format(method, cls)

def test_mut_immut_have_same_methods():
    '''Both modules morphio and morphio.mut should expose the same API'''
    def methods(cls):
        return set(method for method in dir(cls) if not method[:2] == '__')

    only_in_immut = {'section_types', 'diameters', 'perimeters', 'points', 'section_offsets',
                     'as_mutable'}
    only_in_mut = {'remove_unifurcations', 'write', 'append_root_section', 'delete_section', 'build_read_only',
                   'as_immutable'}
    assert (methods(morphio.Morphology) - only_in_immut ==
                 methods(morphio.mut.Morphology) - only_in_mut)

    assert (methods(morphio.Section) ==
                 methods(morphio.mut.Section) - {'append_section'})

    assert (methods(morphio.Soma) ==
                 methods(morphio.mut.Soma))

    assert (methods(morphio.EndoplasmicReticulum) ==
                 methods(morphio.mut.EndoplasmicReticulum))

    # assert (methods(morphio.Mitochondria),
    #              methods(morphio.mut.Mitochondria))
