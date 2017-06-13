#!/usr/bin/env python
from setuptools import setup, Distribution

VERSION = '0.0.0'


class BinaryDistribution(Distribution):
    def is_pure(self):
        return False

    def has_ext_modules(self):
        return True


setup(
    name='brain',
    version=VERSION,
    description='brain',
    packages=['brain',
              'brain.neuron',
              ],
    package_data={
        '': [
            '*.so*',
        ]
    },
    distclass=BinaryDistribution
)
