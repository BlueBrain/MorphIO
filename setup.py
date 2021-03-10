'''morphio setup.py

It is more or less a wrapper to call 'cmake' and 'cmake --build'
'''
import os
import platform
import re
import subprocess
import sys
from distutils.version import LooseVersion

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


MIN_CPU_CORES = 2


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


def find_cmake():
    for candidate in ['cmake', 'cmake3']:
        try:
            out = subprocess.check_output([candidate, '--version'])
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)',
                                                   out.decode()).group(1))
            if cmake_version >= '3.2.0':
                return candidate
        except OSError:
            pass

    raise RuntimeError("CMake >= 3.2.0 must be installed to build MorphIO")


def get_cpu_count():
    try:
        return len(os.sched_getaffinity(0))  # linux only
    except:
        pass

    try:
        return os.cpu_count()  # python 3.4+
    except:
        return 1  # default


class CMakeBuild(build_ext):
    user_options = build_ext.user_options + [
        ("cmake-defs=", None, "Additional CMake definitions, comma split")
    ]

    def initialize_options(self):
        build_ext.initialize_options(self)
        self.cmake_defs = None

    def run(self):
        cmake = find_cmake()
        for ext in self.extensions:
            self.build_extension(ext, cmake)

    def build_extension(self, ext, cmake):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DMORPHIO_VERSION_STRING=' + self.distribution.get_version(),
                      '-DPYTHON_EXECUTABLE=' + sys.executable,
                      '-DHIGHFIVE_EXAMPLES=OFF',
                      '-DHIGHFIVE_UNIT_TESTS=OFF',
        ]

        if self.cmake_defs:
            cmake_args += ["-D" + opt for opt in self.cmake_defs.split(",")]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(
                cfg.upper(),
                extdir)]
            cmake_args += ['-G', os.getenv('MORPHIO_CMAKE_GENERATOR',
                                           "Visual Studio 15 2017 Win64")]
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE={}'.format(cfg),
                           '-DMorphIO_CXX_WARNINGS=OFF',
                           ]
            build_args += ["--", "-j{}".format(max(MIN_CPU_CORES, get_cpu_count())),
                           ]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call([cmake, ext.sourcedir] + cmake_args, cwd=self.build_temp)
        subprocess.check_call([cmake, '--build', '.', '--target', '_morphio'] + build_args, cwd=self.build_temp)

with open('README.rst') as f:
    long_description = f.read()

setup(
    name='MorphIO',
    author='Blue Brain Project, EPFL',
    description='A neuron morphology IO library',
    long_description=long_description,
    long_description_content_type='text/x-rst',
    install_requires=['numpy>=1.14.1'],
    extras_require={
        'docs': ['sphinx-bluebrain-theme'],
    },
    url='https://github.com/BlueBrain/MorphIO/',
    ext_modules=[CMakeExtension('morphio._morphio')],
    cmdclass=dict(build_ext=CMakeBuild),
    packages=['morphio', 'morphio.mut', 'morphio.vasculature'],
    license="LGPLv3",
    keywords=('computational neuroscience',
              'morphology',
              'neuron'
              'neurolucida'
              'neuromorphology'),
    zip_safe=False,
    classifiers=[
        "License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)",
    ],
    use_scm_version=True,
    setup_requires=['setuptools_scm'],
)
