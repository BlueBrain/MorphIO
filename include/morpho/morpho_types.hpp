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
#ifndef MORPHO_TYPES_HPP
#define MORPHO_TYPES_HPP

#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <hadoken/format/format.hpp>
#include <hadoken/geometry/geometry.hpp>

namespace morpho {

// boost ublas alias
typedef boost::numeric::ublas::matrix<double> mat_points;
typedef boost::numeric::ublas::matrix_range<mat_points> mat_range_points;

typedef boost::numeric::ublas::matrix<int> mat_index;

typedef boost::numeric::ublas::vector<double> vec_double;
typedef boost::numeric::ublas::vector_range<vec_double> vec_double_range;

// boost geometry type alias + hadoken extensions
namespace hg = hadoken::geometry::cartesian;
using range = boost::numeric::ublas::range;
using point = hg::point3d;
using vector = hg::vector3d;
using linestring = hg::linestring3d;
using sphere = hg::sphere3d;
using cone = hg::cone3d;
using circle = hg::circle3d;
using circle_pipe = std::vector<circle>;
using box = hg::box3d;

} // morpho

#endif // MORPHO_TYPES_HPP
