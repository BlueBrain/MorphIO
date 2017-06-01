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

class TestCircuitFunctions(unittest.TestCase):
    def setUp(self):
        self.circuit = brain.Circuit(brain.test.circuit_config)

    def test_afferent(self):
        gids = [340, 350]
        synapses = self.circuit.afferent_synapses(gids)
        assert(len(synapses) == 312)
        synapses = self.circuit.afferent_synapses(gids,
                                                  brain.SynapsePrefetch.all)
        assert(len(synapses) == 312)

    def test_efferent(self):
        gids = [340, 350]
        synapses = self.circuit.efferent_synapses(gids)
        assert(len(synapses) == 331)
        synapses = self.circuit.efferent_synapses(gids,
                                                  brain.SynapsePrefetch.all)
        assert(len(synapses) == 331)

    def test_projection(self):
        pre = self.circuit.gids("Layer5")
        post = self.circuit.gids("Layer2")
        synapses = self.circuit.projected_synapses(pre, post)
        print(len(synapses) == 1888)
        synapses = self.circuit.projected_synapses(pre, post,
                                                   brain.SynapsePrefetch.all)
        assert(len(synapses) == 1888)

class TestSynapsesAccessors(unittest.TestCase):

    def setUp(self):
        self.circuit = brain.Circuit(brain.test.circuit_config)
        self.synapses = self.circuit.afferent_synapses([320])

    def test_iterator(self):
        synapses = [s for s in self.synapses]
        assert(len(synapses) == len(self.synapses))

    def test_getitem(self):
         synapse = self.synapses[0]
         synapse = self.synapses[-1]
         size = len(self.synapses)
         synapse = self.synapses[size - 1]
         synapse = self.synapses[-size]
         self.assertRaises(IndexError, lambda: self.synapses[-size - 1])
         self.assertRaises(IndexError, lambda: self.synapses[size])

class TestSynapseArrays(unittest.TestCase):
    def setUp(self):
        self.circuit = brain.Circuit(brain.test.circuit_config)
        self.synapses = self.circuit.afferent_synapses([320])

    def test_arrays(self):
        size = len(self.synapses)

        for function in ["pre_gids", "pre_section_ids", "pre_segment_ids",
                         "pre_distances", "pre_surface_x_positions",
                         "pre_surface_y_positions", "pre_surface_z_positions",
                         "pre_center_x_positions", "pre_center_y_positions",
                         "pre_center_z_positions",
                         "post_gids", "post_section_ids", "post_segment_ids",
                         "post_distances", "post_surface_x_positions",
                         "post_surface_y_positions", "post_surface_z_positions",
                         "post_center_x_positions", "post_center_y_positions",
                         "post_center_z_positions",
                         "delays", "conductances", "utilizations", "depressions",
                         "facilitations", "decays", "efficacies"]:
            array = getattr(self.synapses, function)()
            assert(array.shape == (size,))
        # This data is not available in the test dataset, so we only check the
        # function exists
        self.assertRaises(RuntimeError, lambda: self.synapses.indices())

class TestSynapse(unittest.TestCase):

    def setUp(self):
        self.circuit = brain.Circuit(brain.test.circuit_config)
        self.synapses = self.circuit.afferent_synapses([320])
        self.synapse = self.synapses[0]

    def test_synapse(self):
        # This data is not available in the test dataset, so we only check the
        # function exists
        self.assertRaises(RuntimeError, lambda: self.synapse.gid())

        pre_gid = self.synapse.pre_gid()
        pre_section_id = self.synapse.pre_section()
        pre_segment_id = self.synapse.pre_segment()
        pre_distance = self.synapse.pre_distance()
        pre_surface_position = self.synapse.pre_surface_position()
        pre_center_position = self.synapse.pre_center_position()
        post_gid = self.synapse.post_gid()
        post_section_id = self.synapse.post_section()
        post_segment_id = self.synapse.post_segment()
        post_distance = self.synapse.post_distance()
        post_surface_position = self.synapse.post_surface_position()
        post_center_position = self.synapse.post_center_position()
        delay = self.synapse.delay()
        conductance = self.synapse.conductance()
        utilization = self.synapse.utilization()
        depression = self.synapse.depression()
        facilitation = self.synapse.facilitation()
        decay = self.synapse.decay()
        efficacy = self.synapse.efficacy()

class TestMemoryManagement(unittest.TestCase):

    def test_synapses(self):
        circuit = brain.Circuit(brain.test.circuit_config)
        synapses = circuit.afferent_synapses([320])
        del circuit
        delays = synapses.delays()
        for delay in delays:
            assert(delay < 10)

    def test_synapse(self):
        circuit = brain.Circuit(brain.test.circuit_config)
        synapses = circuit.afferent_synapses([320])
        del circuit
        synapses = [s for s in synapses]
        for synapse in synapses:
            assert(synapse.delay() < 10)

if __name__ == '__main__':
    unittest.main()


