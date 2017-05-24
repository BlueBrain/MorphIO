#!/usr/bin/env python

"""A setuptools based setup module."""
import os.path
from distutils.core import setup, Extension


MORPHOTOOL_EXT = Extension('morphotool.morphotool',
    libraries = ['morpho'],
    sources = ['morphotool/morphotool.cpp'],
    extra_compile_args=['-std=c++11'],
    include_dirs=[]
)

setup_opts = dict(
    name         = 'morphotool',
    version      = '0.4',
    author       = 'EPFL - Blue Brain Project',
    author_email = 'bbp-ou-hpc@groupes.epfl.ch',
    url          = '',
    description  = 'Neuronal Morphology tool',
    download_url = '',
    platforms    = ['Mac OS X', 'Linux'],
    license      = 'GNU General Public License Version 3.0',
    packages     = ["morphotool"],
    ext_modules  = [MORPHOTOOL_EXT]
)


if __name__ == '__main__':
    setup(**setup_opts)
