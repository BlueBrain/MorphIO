#pragma once

#include <array>
#include <iostream>
#include <vector>

namespace morphio {
using Point = std::array<float, 3>;
using Points = std::vector<Point>;

Point operator+(const Point& left, const Point& right);
Point operator-(const Point& left, const Point& right);
Point operator+=(Point& left, const Point& right);
Point operator-=(Point& left, const Point& right);
Point operator/=(Point& left, const float factor);

std::vector<Point> operator+(const std::vector<Point>& points,
    const Point& right);
std::vector<Point> operator-(const std::vector<Point>& points,
    const Point& right);
std::vector<Point> operator+=(std::vector<Point>& points, const Point& right);
std::vector<Point> operator-=(std::vector<Point>& points, const Point& right);

template <typename T>
Point operator*(const Point& from, T factor);
template <typename T>
Point operator*(T factor, const Point& from);
template <typename T>
Point operator/(const Point& from, T factor);
template <typename T>
const Point centerOfGravity(const T& points);

extern template const Point centerOfGravity(const std::vector<std::array<float, 3>>&);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const std::vector<Point>& point);


char my_tolower(char ch);


/**
   Euclidian distance between two points
**/
float distance(const Point& left, const Point& right);

std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os,
    const std::vector<morphio::Point>& points);

} // namespace morphio
std::ostream& operator<<(std::ostream& os, const morphio::Point& point);
std::ostream& operator<<(std::ostream& os,
    const std::vector<morphio::Point>& points);

namespace std {
template <typename T, size_t N>
string to_string(const array<T, N>& a);

} // namespace std
