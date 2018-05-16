#pragma once

#include <array>
#include <iostream>
#include <vector>

namespace morphio
{
typedef std::array<int, 2> Vector2i;
typedef std::array<double, 3> Vector3d;
typedef std::array<float, 3> Vector3f;
typedef std::array<double, 4> Vector4d;
typedef std::array<float, 4> Vector4f;
typedef std::array<std::array<float, 4>, 4> Matrix4f;

typedef std::array<float, 3> Point;
typedef std::vector<Point> Points;

Point operator*(const Point &from, float factor);
Point operator+(const Point &left, const Point &right);
Point operator-(const Point &left, const Point &right);
Point operator+=(Point &left, const Point &right);
Point operator/=(Point &left, float factor);

template <typename T> const Point centerOfGravity(const T& points);

std::string dumpPoint(const Point& point);
std::string dumpPoints(const std::vector<Point>& point);

}
