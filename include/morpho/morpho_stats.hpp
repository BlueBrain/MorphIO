/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */
#ifndef MORPHO_STATS_HPP
#define MORPHO_STATS_HPP

#include <string>
#include <vector>

#include "morpho_tree.hpp"

namespace morpho {

namespace stats {

///
/// \brief return total number of sectiones associated with a given morpho_tree
///
std::size_t total_number_sectiones(const morpho_tree& tree);

///
/// \brief return total number of points associated with a given morpho_tree
///
std::size_t total_number_point(const morpho_tree& tree);

///
/// \brief minimum radius of all the cones / segment of the morphology
///  the soma is not taken into consideration
double min_radius_segment(const morpho_tree& tree);

///
/// \brief max radius of all the cones / segment of the morphology
///  the soma is not taken into consideration
double max_radius_segment(const morpho_tree& tree);

///
/// \brief median radius of all the cones / segment of the morphology
///  the soma is not taken into consideration
double median_radius_segment(const morpho_tree& tree);

///
/// \brief return true if duplicated points has been seen in any section
/// or between the end of a section and the beginning of the next section
///
bool has_duplicated_points(const morpho_tree& tree);

} // stats

} // morpho

#endif // MORPHO_TREE_HPP
