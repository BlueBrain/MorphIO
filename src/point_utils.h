/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/types.h>

#include <iosfwd>  // std::ostream
#include <string>  // std::string


namespace morphio {
Point subtract(const Point& left, const Point& right);

Point centerOfGravity(const range<const Point>& points);

floatType maxDistanceToCenterOfGravity(const Points& points);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const range<const Point>& points);

floatType euclidean_distance(const Point& left, const Point& right);

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const morphio::Points& points);
std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points);
