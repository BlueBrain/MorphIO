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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef MORPHO_TRANSFORM_FILTERS_BITS_HPP
#define MORPHO_TRANSFORM_FILTERS_BITS_HPP

#include <morpho/morpho_transform_filters.hpp>

namespace morpho{



filter_duplicate_point_operation::filter_duplicate_point_operation(double min_distance) : _min_distance(min_distance) {}


std::string filter_duplicate_point_operation::name(){
    return "filter_duplicate";
}




} // morpho



#endif // MORPHO_TRANSFORM_FILTERS_HPP
