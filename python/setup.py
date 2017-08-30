#!/usr/bin/env python
import sys
from setuptools import setup, Extension

morphotool_ext = Extension(
    'morphotool.morphotool',
    libraries=['morpho_io'],
    sources=['morphotool/morphotool.cpp'],
    extra_compile_args=['-std=c++11', '-Wno-unused-local-typedefs'],
    include_dirs=[]
)

setup_opts = dict(
    name         = 'morphotool',
    author       = 'EPFL - Blue Brain Project',
    author_email = 'bbp-ou-hpc@groupes.epfl.ch',
    url          = '',
    description  = 'Neuronal Morphology tool',
    download_url = '',
    platforms    = ['Mac OS X', 'Linux'],
    license      = 'GNU General Public License Version 3.0',
    packages     = ["morphotool"],
    ext_modules  = [morphotool_ext],

    use_scm_version={'root': '..', 'relative_to': __file__},
    install_requires=['numpy'],
    setup_requires=['setuptools_scm']
)


if __name__ == '__main__':
    setup(**setup_opts)
