from nose.tools import ok_
import morphio


def test_doc_exists():
    cls = morphio.Morphology

    classes = [morphio.Morphology,
               # morphio.mut.Morphology,
               # morphio.Section,
               # morphio.mut.Section,
               # morphio.Soma,
               # morphio.mut.Soma,
               # morphio.MitoSection,
               # morphio.mut.MitoSection,
               # morphio.Mitochondria,
               # morphio.mut.Mitochondria
               ]
    for cls in classes:
        public_methods = (method for method in dir(cls) if not method[:2] == '__')
        for method in public_methods:
            ok_(getattr(cls, method).__doc__,
                'Public method {} of class {} is not documented !'.format(method, cls))
