#!/usr/bin/env python

"""A setuptools based setup module."""
import os.path
from distutils.core import setup, Extension


def find_packages(where="."):
    """Short version of setuptools.find_packages"""
    all_pkgs=[]
    for root, dirs, files in os.walk(where, followlinks=True):
        all_dirs = dirs[:]; dirs[:] = []
        for cdir in all_dirs:
            full_path = os.path.join(root, cdir)
            package   = os.path.relpath(full_path, where).replace(os.path.sep, '.')
            if '.' not in cdir and os.path.isfile(os.path.join(full_path, '__init__.py')):
                all_pkgs.append(package)
                dirs.append(cdir)
    return all_pkgs


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
    packages     = find_packages(),
    ext_modules  = [MORPHOTOOL_EXT]
)


if __name__ == '__main__':
    setup(**setup_opts)
