#pragma once

#include <array>
#include <cmath>  // M_PI
#include <iosfwd>  // std::ostream
#include <string>  // std::string
#include <vector>

#include <gsl/gsl-lite.hpp>

namespace morphio {

template <typename T>
using range = gsl::span<T>;

#ifdef MORPHIO_USE_DOUBLE
using floatType = double;
constexpr floatType epsilon = 1e-6;
constexpr floatType PI = M_PI;
#else
/** Type of float to use. Can be double or float depending on MORPHIO_USE_DOUBLE */
using floatType = float;
/** A really small value that is used to measure how close are two values */
constexpr floatType epsilon = 1e-6f;
constexpr floatType PI = static_cast<floatType>(M_PI);
#endif

/** An array of size 3 for x,y,z coordinates */
using Point = std::array<morphio::floatType, 3>;
/** An array of points */
using Points = std::vector<Point>;

Point subtract(const Point& left, const Point& right);

template <typename T>
Point centerOfGravity(const T& points);

template <typename T>
floatType maxDistanceToCenterOfGravity(const T& points);

extern template Point centerOfGravity(const Points&);
extern template floatType maxDistanceToCenterOfGravity(const Points&);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const Points& points);
std::string dumpPoints(const morphio::range<const morphio::Point>& points);

/**
   Euclidian distance between two points
**/
floatType distance(const Point& left, const Point& right);

}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const morphio::Points& points);
std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points);
