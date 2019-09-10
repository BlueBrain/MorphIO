'''morphio setup.py'''
import os
import platform
import re
import subprocess
import sys
from distutils.version import LooseVersion

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


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


class CMakeBuild(build_ext):
    def run(self):
        cmake = find_cmake()
        for ext in self.extensions:
            self.build_extension(ext, cmake)

    def build_extension(self, ext, cmake):
        print('COUCOUCOUCOU')
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":

            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(
                cfg.upper(),
                extdir)]
#            if sys.maxsize > 2**32:
#                cmake_args += ['-A', 'x64']

            cmake_args += ['-G', "Ninja"]
            cmake_args += ['-DCMAKE_CXX_FLAGS:STRING="-m64 -fdiagnostics-absolute-paths -Wno-c++98-compat /DWIN32 /D_WINDOWS /W3 /GR /EHsc /DH5_BUILT_AS_DYNAMIC_LIB"']
            cmake_args += ['-DCMAKE_CXX_COMPILER:FILEPATH=C:/PROGRAM FILES (X86)/MICROSOFT VISUAL STUDIO/2019/COMMUNITY/VC/Tools/Llvm/8.0.0/bin/clang-cl.exe']
            cmake_args += ['-DCMAKE_C_COMPILER:FILEPATH=C:/PROGRAM FILES (X86)/MICROSOFT VISUAL STUDIO/2019/COMMUNITY/VC/Tools/Llvm/8.0.0/bin/clang-cl.exe']
            cmake_args += ['-DCMAKE_MAKE_PROGRAM=C:/PROGRAM FILES (X86)/MICROSOFT VISUAL STUDIO/2019/COMMUNITY/COMMON7/IDE/COMMONEXTENSIONS/MICROSOFT/CMAKE/Ninja/ninja.exe']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE={}'.format(cfg),
                           '-DMorphIO_CXX_WARNINGS=OFF']
            build_args += ['--', '-j']

        print('BUILD ARGS: {}'.format(build_args))
        print('CMAKE ARGS: {}'.format(' '.join(cmake_args)))
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        try:
            output = subprocess.check_call([cmake, ext.sourcedir] + cmake_args,
                                           cwd=self.build_temp)
            output = subprocess.check_call([cmake, '--build', '.'] + build_args,
                                           cwd=self.build_temp)
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
