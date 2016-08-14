# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Juan Hernando <jheranando@fi.upm.es>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>

#!/usr/bin/env python

import h5py
import numpy

SOMA = 1
AXON = 2
BASAL = 3
APICAL = 4

out = h5py.File('test_neuron.h5', 'w')

def createTBranchPoints():
    # Creates a branch with a T shape
    # The first section is a non-homogeneous distance sampling of 11 points
    # on a straight line from (0, 0, 0) to (0, 5, 5).
    first = [[0, i*i / 20.0, i*i / 20.0, 0.5 + i*i/1000.0] for i in range(11)]
    # The second section is an homogeneous sampling from (0, 5, 5) to (-2, 5, 5)
    second = [[-i * 0.4, 5, 5, 0.6 + i/50.0] for i in range(6)]
    # The third section is an homogeneous sampling from (0, 5, 5) to (2, 5, 5)
    third = [[i * 0.4, 5, 5, 0.6 + i/50.0] for i in range(6)]
    return numpy.array(first + second + third)

def createTBranchStructure(firstPoint, nextSection, sectionType):
    return [[firstPoint, sectionType, 0], # connected to soma
            [firstPoint + 11, sectionType, nextSection],
            [firstPoint + 17, sectionType, nextSection]]

points = numpy.array([[.1, .0, .0, .1], [.0, .1, .0, .1],
                      [-.1, .0, .0, .1], [.0, -.1, .0, .1]])
structure = numpy.array([[0, 1, -1]])

# Axon section to the bottom
structure = numpy.append(
    structure, createTBranchStructure(points.shape[0], structure.shape[0], AXON),
    axis=0)
points = numpy.append(
    points, [[p[0], -p[1], p[2], p[3]] for p in createTBranchPoints()], axis=0)
# Basal dendrite to the right
structure = numpy.append(
    structure,
    createTBranchStructure(points.shape[0], structure.shape[0], BASAL), axis=0)
points = numpy.append(
    points, [[p[1], p[0], p[2], p[3]] for p in createTBranchPoints()], axis=0)
# Basal dendrite to the left
structure = numpy.append(
    structure,
    createTBranchStructure(points.shape[0], structure.shape[0], BASAL), axis=0)
points = numpy.append(
    points, [[-p[1], p[0], p[2], p[3]] for p in createTBranchPoints()], axis=0)
# Apical dendrite to the top
structure = numpy.append(
    structure,
    createTBranchStructure(points.shape[0], structure.shape[0], APICAL), axis=0)
points = numpy.append(points, createTBranchPoints(), axis=0)

out.create_dataset('/points', data=points)
out.create_dataset('/structure', data=structure)

out.close()


