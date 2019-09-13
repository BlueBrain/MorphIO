#pragma once

#include <array>
#include <iostream>
#include <string>  // std::string
#include <vector>

namespace morphio {
using Point = std::array<float, 3>;
using Points = std::vector<Point>;

Point operator+(const Point& left, const Point& right);
Point operator-(const Point& left, const Point& right);
Point operator+=(Point& left, const Point& right);
Point operator-=(Point& left, const Point& right);
Point operator/=(Point& left, const float factor);

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
const Point centerOfGravity(const T& points);
template <typename T>
float maxDistanceToCenterOfGravity(const T& points);

extern template const Point centerOfGravity(const Points&);
extern template float maxDistanceToCenterOfGravity(const Points&);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const Points& point);


char my_tolower(char ch);

/**
   Euclidian distance between two points
**/
float distance(const Point& left, const Point& right);

std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const Points& points);

} // namespace morphio
std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os, const morphio::Points& points);

namespace std {
template <typename T, size_t N>
string to_string(const array<T, N>& a)
{
    string res;
    for (auto el : a)
        res += to_string(el) + ", ";
    return res;
}

} // namespace std
