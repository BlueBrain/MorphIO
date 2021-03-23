#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>  // std::string
#include <vector>

namespace morphio {
#ifdef MORPHIO_USE_DOUBLE
using floatType = double;
constexpr floatType epsilon = 1e-6;
constexpr floatType PI = M_PI;
#else
using floatType = float;
constexpr floatType epsilon = 1e-6f;
constexpr floatType PI = static_cast<floatType>(M_PI);
#endif

using Point = std::array<morphio::floatType, 3>;
using Points = std::vector<Point>;

Point operator+(const Point& left, const Point& right);
Point operator-(const Point& left, const Point& right);
Point operator+=(Point& left, const Point& right);
Point operator-=(Point& left, const Point& right);
Point operator/=(Point& left, const floatType factor);

Points operator+(const Points& points, const Point& right);
Points operator-(const Points& points, const Point& right);
Points operator+=(Points& points, const Point& right);
Points operator-=(Points& points, const Point& right);

template <typename T>
Point operator*(const Point& from, T factor);
template <typename T>
Point operator*(T factor, const Point& from);
template <typename T>
Point operator/(const Point& from, T factor);
template <typename T>
Point centerOfGravity(const T& points);
template <typename T>
floatType maxDistanceToCenterOfGravity(const T& points);

extern template Point centerOfGravity(const Points&);
extern template floatType maxDistanceToCenterOfGravity(const Points&);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const Points& point);


char my_tolower(char ch);

/**
   Euclidian distance between two points
**/
floatType distance(const Point& left, const Point& right);

std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const Points& points);

}  // namespace morphio
std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const morphio::Points& points);
