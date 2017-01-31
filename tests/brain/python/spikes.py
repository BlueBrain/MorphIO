# Copyright (c) 2017, EPFL/Blue Brain Project
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

import os
import numpy
import brain

import unittest

class TestSpikeReportReader(unittest.TestCase):

    def setUp(self):
        self.filename = brain.test.root_data_path + \
                        "/local/simulations/may17_2011/Control/out.spikes"

    def test_creation_bad(self):
        self.assertRaises(RuntimeError, lambda: brain.SpikeReportReader("foo"))

    def test_creation(self):
        reader = brain.SpikeReportReader(self.filename)

    def test_get_spikes_bad(self):
        reader = brain.SpikeReportReader(self.filename)
        self.assertRaises(RuntimeError, lambda: reader.get_spikes(0, 0))
        self.assertRaises(RuntimeError, lambda: reader.get_spikes(0, -1))

    def test_get_spikes(self):
        reader = brain.SpikeReportReader(self.filename)
        spikes = reader.get_spikes(-10, 0)
        assert(len(spikes) == 0)

        spikes = reader.get_spikes(1, 5)
        assert(spikes[0][0] >= 1)
        assert(spikes[-1][0] < 5)

        spikes = reader.get_spikes(7, 9)
        assert(spikes[0][0] >= 7)
        assert(spikes[-1][0] < 9)

    def test_properties(self):
        reader = brain.SpikeReportReader(self.filename)
        # assertAlmostEqual fails due to a float <-> double conversion error
        assert(round(reader.end_time - 9.975, 6) == 0)

        # Until some read operation beyond the end is performed the report is
        # not ended
        assert(not reader.has_ended)
        # numpy.nextafter(reader.end_time) should be sufficient, but the result
        # can't be distinguished from end_time after demoted to float in the C++
        # side.
        spikes = reader.get_spikes(0, float("inf"))
        assert(reader.has_ended)

if __name__ == '__main__':
    unittest.main()
