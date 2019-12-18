'''morphio setup.py'''
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
    def run(self):
        cmake = find_cmake()
        for ext in self.extensions:
            self.build_extension(ext, cmake)

    def build_extension(self, ext, cmake):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DMORPHIO_VERSION_STRING=' + self.distribution.get_version(),
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(
                cfg.upper(),
                extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE={}'.format(cfg),
                           '-DMorphIO_CXX_WARNINGS=OFF',
                           ]
            build_args += ["--", "-j{}".format(max(MIN_CPU_CORES, get_cpu_count())),
                           ]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        try:
            subprocess.check_call([cmake, ext.sourcedir] + cmake_args, cwd=self.build_temp)
            subprocess.check_call([cmake, '--build', '.', '--target', 'morphio'] + build_args, cwd=self.build_temp)
        except subprocess.CalledProcessError as exc:
            print("Status : FAIL", exc.returncode, exc.output)
            raise

with open('README.md') as f:
    long_description = f.read()

setup(
    name='MorphIO',
    author='NSE Team - Blue Brain Project',
    author_email='bbp-ou-nse@groupes.epfl.ch',
    description='A neuron morphology IO library',
    long_description=long_description,
    long_description_content_type="text/markdown",
    install_requires=['numpy>=1.14.1'],
    url='https://github.com/BlueBrain/MorphIO/',
    ext_modules=[CMakeExtension('morphio')],
    cmdclass=dict(build_ext=CMakeBuild),
    license="BBP-internal-confidential",
    keywords=('computational neuroscience',
              'morphology',
              'neuron'
              'neurolucida'
              'neuromorphology'),
    zip_safe=False,
    classifiers=[],
    use_scm_version=True,
    setup_requires=['setuptools_scm'],
)
