'''morphio setup.py'''
import os
import platform
import subprocess
import sys

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    user_options = build_ext.user_options + [
        ("cmake-defs=", None, "Additional CMake definitions, comma split")
    ]

    def initialize_options(self):
        build_ext.initialize_options(self)
        self.cmake_defs = None

    def run(self):
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DMORPHIO_VERSION_STRING=' + self.distribution.get_version(),
                      '-DMORPHIO_TESTS=OFF',
                      '-DPYTHON_EXECUTABLE=' + sys.executable,
                      '-DHIGHFIVE_EXAMPLES=OFF',
                      '-DHIGHFIVE_UNIT_TESTS=OFF',
        ]

        if self.cmake_defs:
            cmake_args += ["-D" + opt for opt in self.cmake_defs.split(",")]

        cfg = 'Debug' if self.debug else 'Release'

        build_args = ['--config', cfg]

        cmake_args += ['-DCMAKE_BUILD_TYPE={}'.format(cfg),
                       '-DMorphIO_CXX_WARNINGS=OFF',
                       '-GNinja',
                       ]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp)
        subprocess.check_call(['cmake', '--build', '.', '--target', '_morphio'] + build_args, cwd=self.build_temp)

install_requires = ['numpy>=1.14.1',
                    ]

with open('README.rst', 'r', encoding='utf-8') as f:
    long_description = f.read()

if platform.system() == 'Windows':
    install_requires += ['h5py>=3,<4',   # use h5py's hdf5 install so we don't have to redistribute hdf5
                         ]

setup(
    name='MorphIO',
    author='Blue Brain Project, EPFL',
    description='A neuron morphology IO library',
    long_description=long_description,
    long_description_content_type="text/x-rst",
    install_requires=install_requires,
    extras_require={
        'docs': ['sphinx-bluebrain-theme'],
    },
    url='https://github.com/BlueBrain/MorphIO/',
    ext_modules=[CMakeExtension('morphio._morphio'),
                 ],
    cmdclass={'build_ext': CMakeBuild,
              },
    packages=['morphio', 'morphio.mut', 'morphio.vasculature'],
    license="Apache License 2.0",
    keywords=['computational neuroscience',
              'morphology',
              'neuron',
              'neurolucida',
              'neuromorphology',
              ],
    zip_safe=False,
    classifiers=[
        "Intended Audience :: Education",
        "Intended Audience :: Science/Research",
        "Programming Language :: Python",
        "Topic :: Scientific/Engineering :: Bio-Informatics",
    ],
    use_scm_version=True,
    setup_requires=[
        'setuptools_scm',
    ],
    python_requires=">=3.8",
)
