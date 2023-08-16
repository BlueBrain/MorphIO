/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>  // std::max
#include <cmath>      // std::sqrt
#include <numeric>    // std::accumulate
#include <sstream>    // ostringstream
#include <string>     // std::string

#include "point_utils.h"

#include <morphio/types.h>

namespace morphio {
Point subtract(const Point& left, const Point& right) {
    Point ret;
    for (size_t i = 0; i < ret.size(); ++i) {
        ret[i] = left[i] - right[i];
    }
    return ret;
}

floatType euclidean_distance(const Point& left, const Point& right) {
    return std::sqrt((left[0] - right[0]) * (left[0] - right[0]) +
                     (left[1] - right[1]) * (left[1] - right[1]) +
                     (left[2] - right[2]) * (left[2] - right[2]));
}

std::string dumpPoint(const Point& point) {
    std::ostringstream oss;
    oss << point[0] << " " << point[1] << " " << point[2];
    return oss.str();
}

std::string dumpPoints(const range<const Point>& points) {
    std::ostringstream oss;
    for (const auto& point : points) {
        oss << dumpPoint(point) << '\n';
    }
    return oss.str();
}

Point centerOfGravity(const range<const Point>& points) {
    Point::value_type x = 0;
    Point::value_type y = 0;
    Point::value_type z = 0;
    const auto count = static_cast<Point::value_type>(points.size());
    for (const auto& point : points) {
        x += point[0];
        y += point[1];
        z += point[2];
    }
    return Point({x / count, y / count, z / count});
}

floatType maxDistanceToCenterOfGravity(const Points& points) {
    const auto c = centerOfGravity(points);
    return std::accumulate(std::begin(points),
                           std::end(points),
                           floatType{0},
                           [&](floatType a, const Point& b) {
                               return std::max(a, euclidean_distance(c, b));
                           });
}

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Point& point) {
    return os << morphio::dumpPoint(point);
}

std::ostream& operator<<(std::ostream& os, const morphio::Points& points) {
    return os << morphio::dumpPoints(points);
}

std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points) {
    return os << morphio::dumpPoints(points);
}
