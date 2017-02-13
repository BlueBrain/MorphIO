# Copyright (c) 2017, EPFL/Blue Brain Project
#                     Juan Hernando <juan.hernando@epfl.ch>
#                     Mohamed-Ghaith Kaabi <mohamed.kaabi@epfl.ch>
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
import numpy
import unittest
from brain import *


report_path = brain.test.root_data_path + "/local/simulations/may17_2011/Control/voltage.h5"

class TestReader(unittest.TestCase):
    def setUp(self):
        self.report  = CompartmentReportReader(report_path)

    def test_metadata(self):
        metadata = self.report.metadata
        assert(metadata['data_unit'] == 'mV')
        assert(metadata['time_unit'] == 'ms')
        assert(metadata['start_time'] == 0.0)
        assert(metadata['end_time'] == 10.0)
        assert(metadata['time_step'] == 0.10000000149011612)

    def test_create_view(self):
        view = self.report.create_view({1,2,3})
        assert(view.gids == {1, 2, 3})

    def test_mapping(self):
        view = self.report.create_view({1,2,3})
        mapping = view.mapping
        assert(mapping.num_compartments(1) == 1)
        assert(mapping.index.tolist() ==
               [(0, 1, 0, 1), (1, 2, 0, 1), (2, 3, 0, 1)])
        assert(mapping.offsets == [[0], [1], [2]])
        assert(mapping.compartment_counts() ==  [[1], [1], [1]])

    def test_frames(self):
        view = self.report.create_view({1,2,3})
        frames = view.load(0.0,0.2)
        assert(len(frames) == 2)
        frames = view.load(0.05,0.25)
        # This window overlaps frames [0, 0.1), [0.1, 0.2), [0.2, 03)
        assert(len(frames) == 3)
        frames = view.load(0.09,0.12)
        # Despite the time window is smaller than the timestep, each edge is
        # falling on a different frame, so we get two frames.
        assert(len(frames) == 2)
        frames = view.load_all()
        assert(len(frames) == 100)
        frame = view.load(0.1)
        assert(frame.timestamp == 0.10000000149011612)
        assert(frame.empty == False)
        assert(frame.data.tolist() ==
               [-65.14350891113281, -65.29447937011719, -65.4448013305664])

class TestMemoryManagement(unittest.TestCase):

    def test_view(self):
        view = CompartmentReportReader(report_path).create_view({1, 2, 3})
        assert(view.gids == {1, 2, 3})

    def test_frame(self):
        frame = CompartmentReportReader(report_path).create_view(
            {1, 2, 3}).load(0.1)
        assert(frame.timestamp == 0.10000000149011612)
        assert(frame.empty == False)
        assert(frame.data.tolist() ==
               [-65.14350891113281, -65.29447937011719, -65.4448013305664])

    def test_frame_data(self):
        data = CompartmentReportReader(report_path).create_view(
            {1, 2, 3}).load(0.1).data
        assert(data.tolist() ==
               [-65.14350891113281, -65.29447937011719, -65.4448013305664])

    def test_mapping(self):

        mapping = CompartmentReportReader(report_path).create_view(
            {1, 2, 3}).mapping
        assert(mapping.num_compartments(1) == 1)
        assert(mapping.index.tolist() ==
               [(0, 1, 0, 1), (1, 2, 0, 1), (2, 3, 0, 1)])

if __name__ == '__main__':
    unittest.main()


