/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

namespace morphio {
namespace mut {

class Morphology;

namespace modifiers {
/**
   Only the first and last points of each sections are kept
**/
void two_points_sections(morphio::mut::Morphology& morpho);

/**
   Remove duplicated points
**/
void no_duplicate_point(morphio::mut::Morphology& morpho);

/**
   Reduce the soma to a sphere placed at the center of gravity of soma points
   and whose radius is the averaged distance between the soma points and the
   center of gravity
**/
void soma_sphere(morphio::mut::Morphology& morpho);

/** Reorders neurites of morphology according to NEURON simulator */
void nrn_order(morphio::mut::Morphology& morpho);

}  // namespace modifiers

}  // namespace mut

}  // namespace morphio
