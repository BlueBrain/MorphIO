# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Juan Hernando <juan.hernando@epfl.ch>
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

from .._brain._neuron import *

import numpy as _np

def compute_morphological_samples(morphology, sections, segments, distances):
    """Compute the 3D position and radius of the morphological samples identified
    by tuples of section, segment and distance from segment's start.

    The input tuples are given as three separate iterables of equal length. The
    result is a numpy array of dtype = float and shape = n x 4, where n is the
    length of input iterables.
    """

    samples = _np.empty((len(sections), 4))

    original_samples = {
        id : morphology.section(int(id)).samples() for id in set(sections)}
    index = 0
    for section, segment, distance in zip(sections, segments, distances):
        section = original_samples[section]

        s = section[segment + 1] - section[segment]
        if not s.any():
            # The points are equal
            samples[index] = section[segment]
            index += 1
            continue

        a = distance / _np.linalg.norm(s)
        samples[index] = (section[segment] * (1 - a) + section[segment + 1] * a)
        index += 1

    return samples
