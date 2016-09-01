# Copyright (c) 2016, EPFL/Blue Brain Project
#                     Juan Hernando <juan.hernando@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License version 3.0 as published
# by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import setup
import brain

import unittest

class TestCircuitOpen(unittest.TestCase):

    def test_bad_open(self):
        self.assertRaises(RuntimeError, lambda: brain.Circuit("foo"))

    def test_open(self):
        circuit = brain.Circuit(brain.test.circuit_config)

class TestCircuit(unittest.TestCase):
    def setUp(self):
        self.circuit = brain.Circuit(brain.test.circuit_config)

    def test_gids(self):
        assert(self.circuit.num_neurons() == 1000)
        gids = self.circuit.gids('Column')
        assert(len(gids) == 1000)
        for gid, index in zip(gids, range(1000)):
            assert(gid == index + 1)

        assert(len(self.circuit.gids()) == 1000)

        assert(len(self.circuit.random_gids(0.1)) == 100)
        assert(len(self.circuit.random_gids(0.1, 'Column')) == 100)

    def test_geometry(self):
        gids = self.circuit.gids()
        transforms = self.circuit.transforms(gids)
        positions = self.circuit.positions(gids)
        rotations = self.circuit.rotations(gids)
        assert(transforms.shape == (1000, 4, 4))
        assert(positions.shape == (1000, 3))
        assert(rotations.shape == (1000, 4))

    def test_load_morphology(self):
        morphologies = self.circuit.load_morphologies(
            [1, 100, 1000], brain.Circuit.Coordinates.local)
        morphologies = self.circuit.load_morphologies(
            [1, 100, 1000], brain.Circuit.Coordinates.global_)

    def test_metypes(self):
        count = self.circuit.num_neurons()
        mnames = self.circuit.morphology_names()
        assert(len(mnames) == 9)
        for t in ['L1_SLAC', 'L23_PC', 'L23_MC', 'L4_PC', 'L4_MC',
                  'L5_TTPC1', 'L5_MC', 'L6_TPC_L1', 'L6_MC']:
            assert(t in mnames)

        gids = self.circuit.gids('L23_PC')
        mtypes = self.circuit.morphology_types(gids)
        assert(len(mtypes) == len(gids))
        for t in mtypes:
            assert(mnames[t] == 'L23_PC')

        enames = self.circuit.electrophysiology_names()
        assert(len(enames) == 2)
        for t in ['cACint', 'cADpyr']:
            assert(t in enames)
        etypes = self.circuit.electrophysiology_types(gids)
        assert(len(etypes) == len(gids))
        for t in etypes:
            assert(enames[t] == 'cADpyr')

    def test_bad_gids(self):
        self.assertRaises(RuntimeError,
                          lambda: self.circuit.load_morphologies(
                              [100000], brain.Circuit.Coordinates.local))

if __name__ == '__main__':
    unittest.main()


