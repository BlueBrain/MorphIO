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
import numpy
from brain.neuron import Morphology

import unittest

morphology_path = setup.test_data_path + "/h5/test_neuron.h5"

class TestConstructors(unittest.TestCase):

    def test_simple(self):
        morphology = Morphology(morphology_path)

    def test_transform(self):
        transform = numpy.array([[1, 0, 0, 7],
                                 [0, 1, 0, 8],
                                 [0, 0, 1, 9],
                                 [0, 0, 0, 1]])
        morphology = Morphology(morphology_path, transform)
        assert(numpy.allclose(morphology.transformation(), transform))

    def test_transform_errors(self):
        self.assertRaises(ValueError,
                          lambda: Morphology(morphology_path, 'foo'))
        transform = numpy.array([[2, 2], [2, 2]])
        self.assertRaises(ValueError,
                          lambda: Morphology(morphology_path, transform))
        transform = numpy.array([[1, 0, 0, 7],
                                 [0, 'a', 0, 8],
                                 [0, 0, 1, 9],
                                 [0, 0, 0, 1]])

    def test_bad(self):
        self.assertRaises(RuntimeError, lambda: Morphology("mars"))

class TestCircuitFunctions(unittest.TestCase):
    def setUp(self):
        self.circuit = brain.Circuit(brain.test.circuit_config)

    def test_uris(self):
        uris = self.circuit.morphology_uris([1, 2, 3])
        for uri in uris:
            Morphology(uri)

    def test_load(self):
        circuit = self.circuit

        def is_centered_at_zero(m):
            return numpy.allclose(m.soma().centroid(), [0, 0, 0], atol=1e-7)

        morphologies = circuit.load_morphologies([1, 2, 3],
                                                 circuit.Coordinates.local)
        for m in morphologies:
            assert(is_centered_at_zero(m))

        morphologies = circuit.load_morphologies([1, 2, 3],
                                                 circuit.Coordinates.global_)
        for m in morphologies:
            assert(not is_centered_at_zero(m))

class TestMorphologyFunctions(unittest.TestCase):
    def setUp(self):
        self.morphology = Morphology(morphology_path)

    def test_array_accessors(self):
        points = self.morphology.points()
        assert(points.shape == (96, 4))
        sections = self.morphology.sections()
        assert(sections.shape == (13, 2))
        section_types = self.morphology.section_types()
        assert(section_types.shape == (13,))

    def test_section(self):
        Type = brain.neuron.SectionType
        neurites = [Type.axon, Type.dendrite, Type.apical_dendrite]

        ids = self.morphology.section_ids([Type.soma])
        assert(ids == [0])
        self.assertRaises(RuntimeError, lambda: self.morphology.section(0))

        ids = self.morphology.section_ids(neurites)
        assert(len(ids) == 12)

        sections = self.morphology.sections([Type.soma])
        assert(len(sections) == 0)
        sections = self.morphology.sections(neurites)
        assert(len(sections) == 12)
        for id in ids:
            self.morphology.section(int(id))
        self.assertRaises(RuntimeError, lambda: self.morphology.section(1234))

    def test_soma(self):
        soma = self.morphology.soma()
        assert(len(soma.profile_points()) == 4)
        for p in soma.profile_points():
            assert(len(p) == 4)
        assert(numpy.isclose(soma.mean_radius(), 0.1))
        assert(soma.centroid() == (0, 0, 0))

class TestMorphologyMemoryManagement(unittest.TestCase):

    def setUp(self):
        self.morphology = Morphology(morphology_path)

    def test_soma(self):
        soma = self.morphology.soma()
        del self.morphology
        points = soma.profile_points()
        sum = numpy.zeros(4)
        for p in points:
            sum += p

    def test_section(self):
        section = self.morphology.section(1)
        del self.morphology
        samples = section.samples()
        sum = numpy.zeros(4)
        for s in samples:
            sum += s

class TestSectionFunctions(unittest.TestCase):
    def setUp(self):
        self.morphology = Morphology(morphology_path)
        self.section = self.morphology.section(1)

    def test_equality(self):
        assert(self.section == self.section)
        assert(self.section == self.morphology.section(1))
        assert(self.section != self.morphology.section(2))

    def test_simple_data(self):
        assert(self.section.id() == 1)
        assert(self.section.type() == brain.neuron.SectionType.axon)
        assert(numpy.isclose(self.section.children()[0].distance_to_soma(),
                             self.section.length()))

    def test_samples(self):
        distances = self.section.sample_distances_to_soma()
        assert(distances.shape == (11,))
        samples = self.section.samples()
        assert(samples.shape == (11, 4))
        samples = self.section.samples([0, 0.5, 1.0])
        assert(samples.shape == (3, 4))

    def test_tree(self):
        assert(self.section.parent() == None)
        children = self.section.children()
        assert(len(children) == 2)
        for child in children:
            assert(child.parent() == self.section)

if __name__ == '__main__':
    unittest.main()
